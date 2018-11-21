#include <stdlib.h>

#include "C64MachineState.h"
#include "Assembler.h"
#include "DebugEvaluators.h"
#include "Profiling.h"
#include "Disassembler.h"
#include "Monitor.h"
#include "HueUtil/ProgramOption.h"

using namespace SASM;

static byte readbyte(FILE *f)
{
  byte res;

  fread(&res, 1, 1, f);
  return res;
}

static word readword(FILE *f)
{
  byte res[2];

  fread(&res, 2, 1, f);
  return (res[0] << 8) | res[1];
}

struct SIDemuContext;

static SIDemuContext*
  s_ActivateNextMon;

struct SIDemuContext {
  FullEmu             m_Emu;
  MemAccessMap *      m_Map;
  Monitor*            m_Mon;
  int                 m_Load;
  int                 m_LoadSize;
  int                 m_Init;
  int                 m_Play;
  int                 m_SubTunes;
  int                 m_PlayTicks;
  int                 m_CurrentTick;
  Hue::Util::String   m_SIDFullPath;
  Hue::Util::String   m_SIDName;

  int cmdMonitorToggle(std::vector<Monitor::Arg> const& args) {
    s_ActivateNextMon = this;
    return 0;
  }

  bool loadSID(const char* filename) {
    FILE* in = fopen(filename, "rb");
    if (!in)
    {
      printf("Error: couldn't open file.\n");
      return false;
    }
    long dataoffset  = 0;
    word subtunes    = 0;
    char header[5] = "----";
    fread(header, 4, 1, in);
    if (strcmp(header, "PSID") == 0 ||
        strcmp(header, "RSID") == 0) {
      // Read interesting parts of the SID header
      fseek(in, 6, SEEK_SET);
      dataoffset   = readword(in);
      m_Load       = readword(in);
      m_Init       = readword(in);
      m_Play       = readword(in);
      m_SubTunes   = readword(in);
      fseek(in, dataoffset, SEEK_SET);
      if (m_Load == 0) {
        m_Load = readbyte(in) | (readbyte(in) << 8);
      }
    } else {
      return false;
    }
    // Load the C64 data
    long loadpos = ftell(in);
    fseek(in, 0, SEEK_END);
    long loadend = ftell(in);
    fseek(in, loadpos, SEEK_SET);
    long m_LoadSize = loadend - loadpos;
    if (m_Load + m_LoadSize >= 0x10000) {
      printf("Error: data continues past end of C64 memory.\n");
      fclose(in);
      return false;
    } 
    fread(&m_Emu.m_Mem[m_Load], m_LoadSize, 1, in);
    fclose(in);

    int profiling_address = m_Load;
    int profiling_size    = m_LoadSize;
    m_Map = new MemAccessMap(profiling_address, profiling_size);
    Profiling::attachMemAccessMap(m_Emu, m_Map, m_Load, m_LoadSize);

    m_SIDFullPath = filename;
    m_SIDName     = filename;
    m_SIDName.truncate_up_to_including_last("/");
    m_SIDName.truncate_up_to_including_last("\\");

    m_Mon = new Monitor(m_Emu);
    m_Mon->addCommandT("-", this, &SIDemuContext::cmdMonitorToggle, "toggle monitor instances", NULL);

    m_PlayTicks = 0;
    m_CurrentTick = 0;
    return true;
  }

  int activateMonitor() {
    printf("Monitor: %s\n", m_SIDFullPath.c_str());
    if (m_Mon->interactive() < 0) {
      exit(0);
    }
    return 0;
  }

  bool init(int subtune, bool startMonitor) {
    printf("%-16s : Load: $%04x Init: $%04x Play: $%04x Subtunes: %d\n", m_SIDFullPath.c_str(), m_Load, m_Init, m_Play, m_SubTunes);
    m_Emu.m_Mem[0x01] = 0x37;
    m_Emu.initCPU(m_Init, subtune, 0, 0);
    m_PlayTicks = 0;

    if (startMonitor) {
      activateMonitor();
    }
    return true;
  }

  bool play(int ticks) {
    m_Emu.initCPU(m_Play, 0, 0, 0);
    m_PlayTicks   = ticks;
    m_CurrentTick = 0;
    return true;
  }

  bool step() {
    if (m_Emu.runCPU() || m_Mon->breakpointPending()) {
      if (m_Mon->breakpointPending()) {
        activateMonitor();
      }
      return true;
    } else {
      if (m_PlayTicks > m_CurrentTick) {
        ++m_CurrentTick;
        if (m_CurrentTick % 50 == 0) {
          printf("%6d ", m_CurrentTick);
          dumpSID();
        }
        m_Emu.initCPU(m_Play, 0, 0, 0);
        return true;
      } else {
        return false;
      }
    }
  }

  void dumpSID() {
    for (int i = 0; i <= 0x18; ++i) {
      printf("%02x ", (int)m_Emu.m_Mem[0xd400 + i]);
    }
    printf("\n");
  }

};

static bool stepContexts(std::vector<SIDemuContext*>& contexts) {
  bool cont = true;
  for (int i = 0; i < (int)contexts.size(); ++i) {
    if (!contexts.at(i)->step()) {
      cont = false;
    }
  }
  return cont;
}

int main(int argc, char** argv) {
  int retcode = 1;
  int seconds = 60 * 10;
  Hue::Util::String::List sidnames;
  std::vector<SIDemuContext*> contexts;

  Hue::Util::FlagOption         startMonitorOption                ("m", "", "Start monitor");
  Hue::Util::IntegerOption      defaultSubtuneOption              ("s", "<0-255>", "Subtune number for player", 0, 0, 255);
  Hue::Util::IntegerOption      verbosityOption                   ("v", "<0-2>", "Program verbosity", 1, 0, 2);
  Hue::Util::FlagOption         helpOption                        ("h", "", "Show help");
  Hue::Util::FlagOption         ignorePCOption                    ("i", "", "Ignore progam counter");
  Hue::Util::ProgramOptionList  options;
  options.push_back(&startMonitorOption);
  options.push_back(&verbosityOption);
  options.push_back(&defaultSubtuneOption);
  options.push_back(&ignorePCOption);
  options.push_back(&helpOption);
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (!options.ParseOption(argv[i] + 1)) {
        return 1;
      }
    } else {
      sidnames.push_back(argv[i]);
      contexts.push_back(new SIDemuContext());
    }
  }
  if (sidnames.size() < 2 || helpOption.Value) {
    Hue::Util::String sProgramName(argv[0]);
    sProgramName.truncate_up_to_including_last("\\");
    sProgramName.truncate_up_to_including_last("/");
    printf("%s v0.1 by Stein Pedersen/Prosonix\n", sProgramName.c_str());
    printf("Usage:\n");
    printf("%s <sid_file_1> <sid_file_2> [options]\n", sProgramName.c_str());
    options.PrintHelp();
    return 1;
  }
  for (int i = 0; i < (int)contexts.size(); ++i) {
    if (!contexts.at(i)->loadSID(sidnames.at(i).c_str())) {
      return 1;
    }
  }
  for (int i = 0; i < (int)contexts.size(); ++i) {
    contexts.at(i)->init(defaultSubtuneOption.Value, startMonitorOption.Value);
  }
  std::vector<int> deltas;
  for (int i = 0; i < (int)contexts.size(); ++i) {
    int delta = contexts.at(i)->m_Init - contexts.at(0)->m_Init;
    deltas.push_back(delta);
  }
  enum { INIT, PLAY } state = INIT;
  int tick = 0;
  while (true) {
    while (stepContexts(contexts)) {
      int currentPC = contexts[0]->m_Emu.PC;
      for (int i = 0; i < (int)contexts.size(); ++i) {
        if (!ignorePCOption.Value) {
          int relativePC = contexts.at(i)->m_Emu.PC - deltas[i];
          if (relativePC != currentPC) {
            printf("%s : PC out of sync at $%04x ($%04x)\n", contexts.at(i)->m_SIDFullPath.c_str(), contexts.at(i)->m_Emu.PC, contexts.at(0)->m_Emu.PC);
            printf("CPU history (%s)\n%s\n", contexts.at(i)->m_SIDFullPath.c_str(), contexts.at(i)->m_Mon->getHistory().join("\n").c_str());
            printf("CPU history (%s)\n%s\n", contexts.at(0)->m_SIDFullPath.c_str(), contexts.at(0)->m_Mon->getHistory().join("\n").c_str());
            return 1;
          }
        }
        for (int r = 0; r <= 0x18; ++r) {
          if (contexts.at(i)->m_Emu.m_Mem[0xd400 + r] != contexts.at(0)->m_Emu.m_Mem[0xd400 + r]) {
            printf("%s : SID register $%04x out of sync at $%04x ($%04x)\n", contexts.at(i)->m_SIDFullPath.c_str(), 0xd400 + i, contexts.at(i)->m_Emu.PC, contexts.at(0)->m_Emu.PC);
            printf("CPU history (%s)\n%s\n", contexts.at(i)->m_SIDFullPath.c_str(), contexts.at(i)->m_Mon->getHistory().join("\n").c_str());
            printf("CPU history (%s)\n%s\n", contexts.at(0)->m_SIDFullPath.c_str(), contexts.at(0)->m_Mon->getHistory().join("\n").c_str());
            return 1;
          }
        }
      }
    }
    if (state == INIT) {
      printf("Init OK. Start playing.\n");
      state = PLAY;
      for (int i = 0; i < (int)contexts.size(); ++i) {
        contexts.at(i)->play(1000); // 50*60*10);
      }
    } else {
      printf("Play OK. Tick count: %d\n", contexts[0]->m_CurrentTick);
      break;
    }
  }

      //  for (int frames = 0; frames < seconds * 50; ++frames) {
      //    // Run the playroutine
      //    emu.initCPU(playaddress, 0, 0, 0);
      //    while (emu.runCPU() || mon.breakpointPending())
      //    {
      //      if (mon.breakpointPending()) {
      //        activateMonitor(mon);
      //      }
      //    }
      //  }
      //}
  return retcode;
}
