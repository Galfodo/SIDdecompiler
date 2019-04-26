
#include <stdlib.h>
#include <limits.h>

#include "C64MachineState.h"
#include "Assembler.h"
#include "DebugEvaluators.h"
#include "Profiling.h"
#include "Disassembler.h"
#include "Monitor.h"
#include "STHubbardRipper.h"
#include "HueUtil/ProgramOption.h"

using namespace SASM;

#define SIDDECOMPILER_NAME "siddecompiler"
#define SIDDECOMPILER_VERSION_STRING "0.8"

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

static word readlongword(FILE *f)
{
  byte res[4];

  fread(&res, 4, 1, f);
  return (res[0] << 24) | (res[1] << 16) | (res[2] << 8) | res[3];
}

int myexit(int retcode) {
  return retcode;
}

static int activateMonitor(Monitor& mon) {
  if (mon.interactive() < 0) {
    exit(myexit(0));
  }
  return 0;
}

int main(int argc, char** argv) {
  int retcode = 1;
  int subtune = 0;
  const char* sidname = NULL;
  Hue::Util::StringOption       outputFileOption                  ("o", "<filename>",     "Output filename. If specified, any existing file will be overwritten", NULL);
  Hue::Util::StringOption       monitorCommandFileOption          ("@", "<filename>",     "Execute monitor commands from file", NULL);
  Hue::Util::FlagOption         commmentUnusedDataOption          ("d", "",               "Comment unused data");
  Hue::Util::FlagOption         commmentUnusedCodeOption          ("c", "",               "Comment unused code");
  Hue::Util::FlagOption         startMonitorOption                ("m", "",               "Start monitor");
  Hue::Util::FlagOption         excludeOutOfBoundsVariablesOption ("V", "",               "Exclude out-of-bounds variables (not zp or stack) from disassembly");
  Hue::Util::FlagOption         allowMultipleInstancesOption      ("M", "",               "Parse multiple player instances in .sid file (applies to Hubbard tunes)");
  Hue::Util::FlagOption         includePlayerOption               ("p", "",               "Make source produce runnable .PRG file");
  Hue::Util::FlagOption         standardEntryPointsOption         ("e", "",               "Make source produce .PRG file with standard entry points (init, play)");
  Hue::Util::IntegerOption      defaultSubtuneOption              ("s", "<0-255>",        "Subtune number for player", 0, 0, 255);
  Hue::Util::IntegerOption      verbosityOption                   ("v", "<0-2>",          "Program verbosity", 1, 0, 2);
  Hue::Util::IntegerOption      relocAddressOption                ("a", "<0000-ffff>",    "Relocation address", 0x1000, 0, 0xffff, 4);
  Hue::Util::IntegerOption      initAddrOption                    ("I", "<0000-ffff>",    "Override init address", 0x0000, 0, 0xffff, 4);
  Hue::Util::IntegerOption      playAddrOption                    ("P", "<0000-ffff>",    "Override play address", 0x0000, 0, 0xffff, 4);
  Hue::Util::IntegerOption      playTicksOption                   ("t", "",               "How many times to call the play routine", 60*10*50, 0, INT_MAX); 
  Hue::Util::FlagOption         helpOption                        ("h", "",               "Show help");
  Hue::Util::FlagOption         forcePageAlignmentOption          ("A", "",               "Force page alignment to be the same as the original SID");
  Hue::Util::FlagOption         relocatableZPOption               ("z", "",               "Create labels for ZP addresses");
  Hue::Util::IntegerOption      relocateZPaddressesOption         ("Z", "<2-255>",        "Relocate ZP addresses", 0, 2, 255, 2);
  Hue::Util::IntegerOption      speculativeCodeOption             ("C", "<0-1>",          "How speculative should the disassembler be about handling unreferenced code/data", 0, 0, 1);
  Hue::Util::IntegerOption      overrideSubTuneCountOption        ("S", "<0-255>",        "Override subtune count found in SID file", 0, 0, 255);
  Hue::Util::FlagOption         singleSubtuneOption               ("1", "",               "Trace only 1 subtune (0 or specified with -s)");
  Hue::Util::FlagOption         reloadTuneOption                  ("r", "",               "Reload tune before disassembling");
  Hue::Util::FlagOption         versionOption                     ("-version", "",        "Print version string");
  Hue::Util::AliasOption        longHelpOption("-help", "The same as -h", &helpOption);
  Hue::Util::ProgramOptionList  options;
  options.push_back(&outputFileOption);
  options.push_back(&monitorCommandFileOption);
  options.push_back(&relocAddressOption);
  options.push_back(&commmentUnusedDataOption);
  options.push_back(&commmentUnusedCodeOption);
  options.push_back(&excludeOutOfBoundsVariablesOption);
  options.push_back(&startMonitorOption);
  options.push_back(&allowMultipleInstancesOption);
  options.push_back(&verbosityOption);
  options.push_back(&includePlayerOption);
  options.push_back(&standardEntryPointsOption);
  options.push_back(&defaultSubtuneOption);
  options.push_back(&forcePageAlignmentOption);
  options.push_back(&speculativeCodeOption);
  options.push_back(&initAddrOption);
  options.push_back(&playAddrOption);
  options.push_back(&playTicksOption);
  options.push_back(&overrideSubTuneCountOption);
  options.push_back(&relocatableZPOption);
  options.push_back(&relocateZPaddressesOption);
  options.push_back(&singleSubtuneOption);
  options.push_back(&reloadTuneOption);
  options.push_back(&helpOption);
  options.push_back(&versionOption);
  options.push_back(&longHelpOption);
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (!options.ParseOption(i, argc, argv)) {
        options.PrintHelp();
        return myexit(1);
      }
    } else {
      sidname = argv[i];
    }
  }
  if (versionOption.Value) {
    printf(SIDDECOMPILER_NAME " v" SIDDECOMPILER_VERSION_STRING "\nCopyright (C) 2017 Stein Pedersen/Prosonix\n");
    return myexit(0);
  }
  if (sidname == NULL || helpOption.Value) {
    printf("Usage:\n");
    printf(SIDDECOMPILER_NAME " <sid_file> [options]\n");
    options.PrintHelp();
    return myexit(0);
  }

  FILE* in = fopen(sidname, "rb");
  if (!in)
  {
    printf("Error: couldn't open file.\n");
    return myexit(retcode);
  }

  bool
    isSID = false;

  long dataoffset     = 0;
  int  loadaddress    = 0;
  word initaddress    = 0;
  word playaddress    = 0;
  word subtunes       = 0;
  int  defaultsubtune = 0;
  int  speedf         = 0;
  char header[5] = "----";
  fread(header, 4, 1, in);
  if (strcmp(header, "PSID") == 0 ||
      strcmp(header, "RSID") == 0) {
    // Read interesting parts of the SID header
    fseek(in, 6, SEEK_SET);
    dataoffset      = readword(in);
    printf("Dataoffset: %04X\n", dataoffset);
    loadaddress     = readword(in);
    initaddress     = readword(in);
    playaddress     = readword(in);
    subtunes        = readword(in);
    defaultsubtune  = readword(in);
    speedf          = readlongword(in);
    fseek(in, dataoffset, SEEK_SET);
    if (loadaddress == 0) {
      int lo = readbyte(in);
      int hi = readbyte(in);
      loadaddress = lo | (hi << 8);
    }
    isSID = true;

  } else {
    printf("File is not SID. Loading as PRG\n");
    fseek(in, 0, SEEK_SET);
    unsigned char buf[2] = "";
    fread(buf, 2, 1, in);
    loadaddress  = buf[0] | (buf[1] << 8);
  }
  if (initAddrOption.Value) {
    initaddress = initAddrOption.Value;
  }
  if (playAddrOption.Value) {
    playaddress = playAddrOption.Value;
  }
  if (overrideSubTuneCountOption.Value > 0) {
    subtunes = overrideSubTuneCountOption.Value;
  }
  // Load the C64 data
  long loadpos = ftell(in);
  fseek(in, 0, SEEK_END);
  long loadend = ftell(in);
  fseek(in, loadpos, SEEK_SET);
  long loadsize = loadend - loadpos;
  if (loadsize + loadaddress >= 0x10000) {
    printf("Error: data continues past end of C64 memory.\n");
    fclose(in);
    return myexit(retcode);
  } else {

    SASM::FullEmu
      emu;
  
    fread(&emu.m_Mem[loadaddress], loadsize, 1, in);
    fclose(in);

    bool
      isHubbard = false;

    STSIDRipper*     sidRipper     = NULL;
    STHubbardRipper* hubbardRipper = NULL;
    if (isSID) {
      sidRipper = new STSIDRipper;
      sidRipper->load(sidname);
      hubbardRipper = new STHubbardRipper(*sidRipper);
      if (hubbardRipper->scanForData()) {
        isHubbard = true;
      }
    }

    int profiling_address = excludeOutOfBoundsVariablesOption.Value ? loadaddress : 0x0200;
    int profiling_size    = excludeOutOfBoundsVariablesOption.Value ? loadsize    : 0xfe00;

    SASM::MemAccessMap
      map(profiling_address, profiling_size);

    Profiling::attachMemAccessMap(emu, &map, loadaddress, loadsize);

    subtunes = subtunes ? subtunes : 1;
    if (isHubbard) {
      assert(hubbardRipper->m_Instances.size());
      printf("%d Rob Hubbard driver instance(s) found. ", (int)hubbardRipper->m_Instances.size());
      if (!allowMultipleInstancesOption.Value) {
        if (hubbardRipper->m_Instances.size() > 1)
          printf("Processing instance 1 only.");
        subtunes = (SASM::word)hubbardRipper->m_Instances[0].m_Songs.size();
      }
      printf("\n");
    }
    Hue::Util::String sSidName = sidname;
    sSidName.truncate_up_to_including_last("/");
    sSidName.truncate_up_to_including_last("\\");
    bool startMonitor = startMonitorOption.Value;
    printf("Loaded '%s'\n\n", sSidName.c_str());

    // Save snapshot of memory
    auto snapshot = emu.getSnapshot();

    Monitor
      mon(emu);

    int reloc_end = loadaddress + loadsize - 1;
    mon.setVariable("reloc_begin", (word*)&loadaddress);
    mon.setVariable("reloc_end",   (word*)&reloc_end);
    mon.setVariable("init",        (word*)&initaddress);
    mon.setVariable("play",        (word*)&playaddress);
    if (!monitorCommandFileOption.Value.empty())
    {
      startMonitorOption.Value = true;
      mon.executeScript(monitorCommandFileOption.Value.c_str());
    }
    if (isSID) {
      printf("Load address: $%04x Init address: $%04x Play address: $%04x Subtunes: %d\n\n", loadaddress, initaddress, playaddress, subtunes);
      mon.setVariable("init", (word*)&initaddress);
      mon.setVariable("play", (word*)&playaddress);
      for (subtune = 0; subtune < subtunes; ++subtune) {
        if (singleSubtuneOption.Value && subtune != defaultSubtuneOption.Value) {
          continue;
        }
        emu.m_Mem[0x01] = 0x37;
        emu.initCPU(initaddress, subtune, 0, 0);
        if (startMonitor) {
          startMonitor = false;
          activateMonitor(mon);
        }
        printf("Emulating subtune %d play\n", subtune);
        while (emu.runCPU() || mon.breakpointPending())
        {
          if (mon.breakpointPending()) {
            activateMonitor(mon);
          }
        }
        for (int frames = 0; frames < playTicksOption.Value; ++frames) {
          // Run the playroutine
          emu.initCPU(playaddress, 0, 0, 0);
          while (emu.runCPU() || mon.breakpointPending())
          {
            if (mon.breakpointPending()) {
              activateMonitor(mon);
            }
          }
        }
      }
    } else {
      printf("Load address: $%04x-$%04x\n\n", loadaddress, loadaddress + (int)loadsize - 1);
      if (initaddress == 0) {
        initaddress = loadaddress;
        playaddress = loadaddress + 3;
      }
      int pc = initaddress;
      if (loadaddress == 0x0801 && emu.getByte(0x0805) == 0x9e) {
        playaddress = 0;
        int sys = atoi((const char*)&emu.m_Mem[0x0806]);
        if (sys >= 2061) {
          pc = sys;
        }
      }
      emu.initCPU(pc, 0, 0, 0);
      if (startMonitor) {
        startMonitor = false;
        activateMonitor(mon);
      }
      while (emu.runCPU() || mon.breakpointPending())
      {
        if (mon.breakpointPending()) {
          activateMonitor(mon);
        }
      }
      if (playaddress) {
        for (int frames = 0; frames < playTicksOption.Value; ++frames) {
          // Run the playroutine
          emu.initCPU(playaddress, 0, 0, 0);
          while (emu.runCPU() || mon.breakpointPending())
          {
            if (mon.breakpointPending()) {
              activateMonitor(mon);
            }
          }
        }
      }
    }
    if (verbosityOption.Value > 1)
      map.dump();

    if (reloadTuneOption.Value) {
      // Restore memory from snapshot. Some tunes are not restartable.
      emu.restoreSnapshot(snapshot);
    }
    delete snapshot;
    snapshot = NULL;

    // Include only accessed memory
    int access_begin = map.m_LowestAccessed;
    int access_end   = map.m_HighestAccessed;
    int access_size  = access_end - access_begin + 1;

    map.traceState().buildRelocationTables(emu, access_begin, access_size);

    Disassembler
      disasm(emu.m_Mem + access_begin, access_begin, access_size, commmentUnusedDataOption.Value, commmentUnusedCodeOption.Value);

    if (relocateZPaddressesOption.Value >= 2) {
      relocatableZPOption.Value = true;
      disasm.m_RelocZPaddr = relocateZPaddressesOption.Value & 0xff;
    }
    disasm.m_RelocZP = relocatableZPOption.Value;
    if (speculativeCodeOption.Value > 0) {
      disasm.m_UnknownAlwaysData = false;
    }

    bool
      isOverwriteExisting = false;

    Hue::Util::String
      sAsmFile(sSidName);

    sAsmFile.truncate_from_last(".");
    sAsmFile.append(".asm");
    if (!outputFileOption.Value.empty()) {
      sAsmFile = outputFileOption.Value;
      isOverwriteExisting = true;
    }
    if (initaddress) {
      disasm.addLabel(initaddress, "init", true);
    }
    if (playaddress) {
      disasm.addLabel(playaddress, "play", true);
    }
    if (isHubbard) {
      for (int instanceIndex = 0; instanceIndex < (int)hubbardRipper->m_Instances.size(); ++instanceIndex) {
        Hue::Util::String sInstancePrefix;
        if (hubbardRipper->m_Instances.size() > 1) {
          sInstancePrefix.printf("i%d_", instanceIndex);
        }
        STHubbardRipper::PlayerInstance& instance = hubbardRipper->m_Instances.at(instanceIndex);

  #define MAKELABEL(str) Hue::Util::String::static_printf("%s%s", sInstancePrefix.c_str(), str).c_str()

        if (instance.m_FrqAddress) {
          auto frqlo = disasm.addLabel(instance.m_FrqAddress,     MAKELABEL("frqlo"), false);
          auto frqhi = disasm.addLabel(instance.m_FrqAddress + 1, MAKELABEL("frqhi"), false);
          disasm.addOffsetAlias(frqlo, 2);
          disasm.addOffsetAlias(frqhi, 2);
          disasm.addOffsetAlias(frqlo, -2);
          disasm.addOffsetAlias(frqhi, -2);
        }
        auto instrLabel = disasm.addLabel(instance.m_InstrumentAddress, MAKELABEL("instr"), false);
        disasm.addOffsetAlias(instrLabel, 1);
        disasm.addOffsetAlias(instrLabel, 2);
        disasm.addOffsetAlias(instrLabel, 3);
        disasm.addOffsetAlias(instrLabel, 4);
        disasm.addOffsetAlias(instrLabel, 5);
        disasm.addOffsetAlias(instrLabel, 6);
        disasm.addOffsetAlias(instrLabel, 7);
        disasm.addLabel(instance.m_SeqLoAddress, MAKELABEL("seqlo"), false);
        disasm.addLabel(instance.m_SeqHiAddress, MAKELABEL("seqhi"), false);
        auto seqtable = disasm.addAddressTable(MAKELABEL("s"), instance.m_SeqLoAddress, instance.m_SeqHiAddress, instance.m_SequencesUsedGuesstimate);
        map.traceState().removeOverlappingRelocationTables(seqtable);
        for (int i = 0; i < instance.m_SequencesUsedGuesstimate; ++i) {
          int seqaddr = disasm.getAddressFromTableAt(seqtable, i);
          if (disasm.isAddressInRange(seqaddr)) {
            auto tmp_lbl = MAKELABEL(Hue::Util::String::static_printf("s%02x", i).c_str());
            //if (strcmp(tmp_lbl, "s4f") == 0) {
            //  int debug = 0;
            //}
            auto seqlabel = disasm.getOrCreateLabel(seqaddr, tmp_lbl , false);
            if (seqlabel->m_Name != tmp_lbl) {
              disasm.addAlias(seqaddr, tmp_lbl);
            }
          }
        }
        assert(instance.m_Songs.size() > 0);
        {
          bool singleTune = instance.m_Songs.size() == 1;
          for (int i = 0; i < (int)instance.m_Songs.size(); ++i) {
            auto song = instance.m_Songs[i];
            auto songlbl = disasm.getOrCreateLabel(song.m_SongAddress, MAKELABEL(Hue::Util::String::static_printf("song%02x", i).c_str()), false);
            if (singleTune) {
              disasm.addOffsetAlias(songlbl, 3);
            }
            auto trktbl = disasm.addAddressTable(MAKELABEL(Hue::Util::String::static_printf("song%02Xtrk", i).c_str()), song.m_SongAddress, song.m_SongAddress + 3, 3);
            map.traceState().removeOverlappingRelocationTables(trktbl);
            for (int trk = 0; trk < 3; ++trk) {
              int trkaddr = disasm.getAddressFromTableAt(trktbl, trk);
              if (disasm.isAddressInRange(trkaddr)) {
                disasm.addLabel(trkaddr, MAKELABEL(Hue::Util::String::static_printf("song%02Xtrk%02x", i, trk).c_str()), false);
              }
            }
          }
        }
      }
    }
    int tblindex = 0;
    for (auto it = map.traceState().relocationTables().begin(); it != map.traceState().relocationTables().end(); ++it, ++tblindex) {
      if (it->m_Size > 1) {
        auto tbl = disasm.addAddressTable(NULL, it->m_LoBytes, it->m_HiBytes, it->m_Size);
        for (int entry = 0; entry < it->m_Size; ++entry) {
          int addr = disasm.getAddressFromTableAt(tbl, entry);
          bool isExec = (map.getAccessType(addr) & MemAccessMap::EXECUTE) ? true : false;
          disasm.getOrCreateLabel(addr, tbl->formatLabel(disasm, addr, entry).c_str(), isExec);
        }
      } else {
        auto tbl = disasm.addRelocAddress(NULL, it->m_LoBytes, it->m_HiBytes);
        int addr = disasm.getAddressFromTableAt(tbl, 0);
        bool isExec = (map.getAccessType(addr) & MemAccessMap::EXECUTE) ? true : false;
        disasm.getOrCreateLabel(addr, tbl->formatLabel(disasm, addr, 0).c_str(), isExec);
      }
    }
    disasm.parseMemAccessMap(&map);
    disasm.disassemble();

    Hue::Util::String sText;
    sText.appendf("; This file was automatically generated by SIDdecompiler " SIDDECOMPILER_VERSION_STRING " from '%s'\n", sidname);
    if (sidRipper) {
      sText.appendf("; Name:      %s\n", sidRipper->m_Header.name.c_str());
      sText.appendf("; Author:    %s\n", sidRipper->m_Header.author.c_str());
      sText.appendf("; Released:  %s\n", sidRipper->m_Header.released.c_str());
      sText.appendf("; Subtunes:  %d\n", subtunes);
    }
    if (map.traceState().m_IncompleteOperands.size()) {
      sText.append(";\n; WARNING: May have alignment issues due to partial address operand modification.\n");
      for (auto it = map.traceState().m_IncompleteOperands.begin(); it != map.traceState().m_IncompleteOperands.end(); ++it) {
        sText.appendf("; Operand at %s\n", disasm.formatAddressUsingClosestLabel(*it, true).c_str());
      }
    }
    sText.append("\n");
    if (includePlayerOption.Value) {
      if (initaddress && playaddress) {
        bool isCIA = (speedf & (1 << defaultSubtuneOption.Value)) ? true : false;
        sText.append(""
          "                * = $0801                                                    \n"
          "                                                                             \n"
          "                .byte $0b,$08,$e1,$07,$9e,$32,$30,$36,$31,0,0,0              \n"
          "                sei                                                          \n"
          "                lda #$35                                                     \n"
          "                sta $01                                                      \n"
          "reinit          lda #$00                                                     \n"
          "                jsr init                                                     \n"
          );
        if (isCIA) {
          sText.append(""
            "loop            lda $dc0d                                                    \n"
            "                and #$01                                                     \n"
            "                beq loop                                                     \n"
            "                jsr play                                                     \n"
          );
        } else {
          sText.append(""
            "loop            bit $d011                                                    \n"
            "                bmi *-3                                                      \n"
            "                bit $d011                                                    \n"
            "                bpl *-3                                                      \n"
            "                inc $d020                                                    \n"
            "                jsr play                                                     \n"
            "                dec $d020                                                    \n"
          );
        }
        sText.append(""
            "                lda $dc01                                                    \n"
            "                and #$10                                                     \n"
            "                tay                                                          \n"
            "                eor #$10                                                     \n"
            "                and dc01_prev                                                \n"
            "                sty dc01_prev                                                \n"
            "                beq loop                                                     \n"
            "                bne reinit                                                   \n"
            "dc01_prev       .byte $10                                                    \n"
            "                                                                             \n"
          );
        if (defaultSubtuneOption.Value >= 0 && defaultSubtuneOption.Value < subtunes) {
          subtune = defaultSubtuneOption.Value;
        }
        printf("Creating source file for runnable .prg with subtune %d\n", subtune);
        sText.replace("#$00", Hue::Util::String::static_printf("#$%02x", subtune).c_str());
      } else {
        sText.append(""
          "                * = $0801                                                    \n"
          "                                                                             \n"
          "                .byte $0b,$08,$e1,$07,$9e,$32,$30,$36,$31,0,0,0              \n"
          "                jmp init                                                     \n");
      }
    } else if (standardEntryPointsOption.Value) {
        sText.append(""
          "                * = $0801                                                    \n"
          "                                                                             \n"
          "                jmp init                                                     \n"
          "                jmp play                                                     \n");
        sText.replace("0801", Hue::Util::String::static_printf("%04x", relocAddressOption.Value));
    }
    if (forcePageAlignmentOption.Value) {
      int value = (loadaddress & 0xff) | 0x0900;
      sText.appendf("                * = $%04x                                                    \n", value);
    }
    disasm.setHeader(sText.c_str());
    auto txt = disasm.dump(relocAddressOption.Value);
    {
      FILE* out = NULL;
      const char* filename = sAsmFile.c_str();
      if (!isOverwriteExisting) {
        out = fopen(filename, "rb");
        if (out) {
          fclose(out);
          fprintf(stderr, "\nERROR: Output file '%s' already exists. Use -o to overwrite.\n", filename);
          return myexit(1);
        }
      }
      if (filename) {
        out = fopen(filename, "wb");
      } else {
        out = stdout;
      }
      if (out) {
        fprintf(out, "%s", txt.c_str());
      }
      if (filename) {
        if (out) {
          printf("Wrote '%s'\n", filename);
          fclose(out);
        } else {
          fprintf(stderr, "ERROR: Failed to open '%s' for writing\n", filename);
        }
      }
    }
    if (map.traceState().m_IncompleteOperands.size()) {
      printf("\nWARNING: Generated source may have alignment issues due to partial address\noperand modification. Use option '-A' to ensure player alignment.\n");
    }
    if (startMonitorOption.Value) {
      mon.interactive();
    }
    map.traceState().cleanup();
    retcode = 0;
  }
  return myexit(retcode);
}
