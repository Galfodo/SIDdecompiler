
#include "STHubbardRipper.h"
#include <assert.h>

STHubbardRipper::STHubbardRipper(STSIDRipper& m_sid) : m_sid(m_sid) {
}

bool STHubbardRipper::scanForData() {
  int foundAddress = 0;
  unsigned char* memory = m_sid.m_Memory.data();
  int search_songs  = 0;
  int search_instr  = 0;
  int search_seq    = 0;
  int search_frq    = 0;
  do {
    PlayerInstance instance;
    // Find pointer to "songs" data section.
    //   lda songs,x       $bd SONGS_LOW SONGS_HIGH
    //   sta currtrkhi,y   $99 * *
    //   inx               $e8
    //   iny               $c8
    //   cpy #$06          $c0 $06
    int songsAddress;
    foundAddress = m_sid.findString("bd****99****e8c8c006", search_songs);
    if (foundAddress > 0)
    {
      search_songs = foundAddress + 1;
      songsAddress = memory[foundAddress + 1] | (memory[foundAddress + 2] << 8);
      // Now songsAddress points to a 6-byte structure a la:
      //   songs:
      //     .dat <montymaintr1, <montymaintr2, <montymaintr3
      //     .dat >montymaintr1, >montymaintr2, >montymaintr3
      while (songsAddress < 65536) {
        SongData song;
        song.m_SongAddress     = songsAddress;
        song.m_TrackAddress[0] = memory[songsAddress]     | (memory[songsAddress + 3] << 8);
        song.m_TrackAddress[1] = memory[songsAddress + 1] | (memory[songsAddress + 4] << 8);
        song.m_TrackAddress[2] = memory[songsAddress + 2] | (memory[songsAddress + 5] << 8);
        if (m_sid.isAddressInSIDRange(song.m_TrackAddress[0]) && m_sid.isAddressInSIDRange(song.m_TrackAddress[1]) && m_sid.isAddressInSIDRange(song.m_TrackAddress[2])) {
          instance.m_Songs.push_back(song);
          songsAddress += 6;
        } else {
          if (instance.m_Songs.empty()) {
            goto not_found;
          }
          break;
        }
      }
    } else {
      // Only one song in m_sid. Has no "songs" data, but hard coded pointers to one song instead.
      // currtrkhi points to the high bytes for each voice, currtrklo to ditto for low bytes.
      // Find currtrkhi/currtrklo.
      //   lda currtrklo,x    $bd CURRTRKLO_LOW CURRTRKLO_HIGH
      //   sta $02            $85 *
      //   lda currtrkhi,x    $bd CURRTRKHI_LOW CURRTRKHI_HIGH
      //   sta $03            $85 *
      //   dec lengthleft,x   $de * *
      foundAddress = m_sid.findString("bd****85**bd****85**de", search_songs);
      if (foundAddress > 0) {
        search_songs = foundAddress + 1;
        int currtrklo = memory[foundAddress + 1] | (memory[foundAddress + 2] << 8);
        int currtrkhi = memory[foundAddress + 6] | (memory[foundAddress + 7] << 8);
        SongData song;
        song.m_SongAddress      = currtrklo;
        if (currtrkhi - currtrklo != 3) {
          goto not_found;
        }
        song.m_TrackAddress[0]  = memory[currtrklo]     | (memory[currtrkhi] << 8);
        song.m_TrackAddress[1]  = memory[currtrklo + 1] | (memory[currtrkhi + 1] << 8);
        song.m_TrackAddress[2]  = memory[currtrklo + 2] | (memory[currtrkhi + 2] << 8);
        if (m_sid.isAddressInSIDRange(song.m_TrackAddress[0]) && m_sid.isAddressInSIDRange(song.m_TrackAddress[1]) && m_sid.isAddressInSIDRange(song.m_TrackAddress[2])) {
          instance.m_Songs.push_back(song);
        } else {
          goto not_found;
        }
      } else {
        goto not_found;
      }
    }

    // Find pointer to "instr" data section.
    //   lda instr,x       $bd INSTR_LOW INSTR_HIGH
    //   sta $d402,y       $99 $02 $d4
    //   lda instr + 1,x   $bd * *
    //   sta $d403,y       $99 $03 $d4
    foundAddress = m_sid.findString("bd****9902d4bd****9903d4", search_instr);
    if (foundAddress <= 0) {
      // Find pointer to "instr" data section. 2nd chance:
      //   lda instr,x       $bd INSTR_LOW INSTR_HIGH
      //   sta $d402,y       $99 $02 $d4
      //   pha               $48
      //   lda instr + 1,x   $bd * *
      //   sta $d403,y       $99 $03 $d4
      foundAddress = m_sid.findString("bd****9902d448bd****9903d4", search_instr);
    }
    if (foundAddress > 0) {
      search_instr = foundAddress + 1;
      instance.m_InstrumentAddress = memory[foundAddress + 1] | (memory[foundAddress + 2] << 8);
    } else {
      goto not_found;
    }

    // Find pointer to "patptl" and "patpth" data sections.
    //   tay               $a8
    //   lda patptl,y      $b9 PATPTL_LOW PATPTL_HIGH
    //   sta $04           $85 *
    //   lda patpth,y      $b9 PATPTH_LOW PATPTH_HIGH
    //   sta $05           $85 *
    foundAddress = m_sid.findString("a8b9****85**b9****85**", search_seq);
    if (foundAddress > 0)
    {
      search_seq = foundAddress + 1;
      instance.m_SeqLoAddress = memory[foundAddress + 2] | (memory[foundAddress + 3] << 8);
      instance.m_SeqHiAddress = memory[foundAddress + 7] | (memory[foundAddress + 8] << 8);
      instance.m_SequencesUsedGuesstimate = instance.m_SeqHiAddress - instance.m_SeqLoAddress;
    }
    int frqAddr = m_sid.findString("16012701", search_frq);
    if (frqAddr > 0) {
      search_frq = frqAddr + 1;
      instance.m_FrqAddress = frqAddr;
    } else {
      instance.m_FrqAddress = 0;
    }
    m_Instances.push_back(instance);
  } while (true);
not_found:
  return m_Instances.size() > 0;
}

//bool STHubbardRipper::convert() {
//  unsigned char* memory = m_sid.m_Memory.data();
//  bool is_converted[256];
//  memset(is_converted, 0, sizeof(is_converted));
//  m_app.song().init();
//  assert(m_app.song().m_SubTunes.size() == 1);
//  assert(m_app.song().m_SubTunes[0].m_Tracks.size() == 3);
//  assert(m_app.song().m_Sequences.size() >= 0x48);
//  for (int voice = 0; voice < 3; ++voice) {
//    int trackAdr = m_TrackAddress[voice];
//    for (int i = 0; i < 256; ++i) {
//      int trackdata = memory[trackAdr + i];
//      m_app.song().tracks()[voice].m_RawData[i] = trackdata;
//      if (trackdata >= 0xfe)
//        break;
//      if (trackdata >= 0x48) {
//        printf("Sequence out of range: %d\n", trackdata);
//        continue;
//      }
//      if (!is_converted[trackdata]) {
//        is_converted[trackdata] = convertSequence(trackdata);
//        if (!is_converted[trackdata])
//          return false;
//      }
//    }
//  }
//  return true;
//}

//bool STHubbardRipper::convertSequence(int sequence) {
//  SteinTronic::Sequence& seq = m_app.song().m_Sequences.at(sequence);
//  unsigned char* memory = m_sid.m_Memory.data();
//  int notelength = 0;
//  int seqAddr = memory[m_SeqLoAddress + sequence] | (memory[m_SeqHiAddress + sequence] << 8);
//  int write = 0;
//  bool tied = false;
//  bool nextTied = false;
//  while (true) {
//    int new_instrument = 0xff;
//    if (!m_sid.isLegalAddress(seqAddr))
//      return false;
//    int data = memory[seqAddr];
//    if (data == 0xff) {
//      seq.terminateAt(write);
//      break;
//    }
//    notelength = ((data & 0x1f) + 1) / m_SpeedDivider;
//    if (notelength == 0)
//    {
////      int whaat = 0;
//    }
//    if (data & 0x40) {
//      seq.setReleaseAt(write, notelength);
//      write += notelength;
//      ++seqAddr;
//      continue;
//    }
//    bool portamento = false;
//    int portaval;
//    nextTied = false;
//    if (data & 0x20)
//      nextTied = true;
//    if (data & 0x80) {
//      ++seqAddr;
//      data = memory[seqAddr];
//      if (data == 0xff) {
//        seq.terminateAt(write);
//        break;
//      }
//      if ((data & 0x80) == 0) {
//        new_instrument = data;
//        if (new_instrument > 0x20)
//        {
////          int hepp = 0;
//        }
//      } else {
//        portamento = true;
//        data = data & 127;
//        if ((data & 1) == 1) {
//          // porta down
//          portaval = -(data >> 1);
//        }
//        else {
//          // porta up
//          portaval = data >> 1;
//        }
//      }
//    }
//    ++seqAddr;
//    data = memory[seqAddr];
//    if (data == 0xff) {
//      seq.terminateAt(write);
//      break;
//    }
//    data = data & 0x7f;
//    if (data > RawNote::LastNote) {
//      seq.setReleaseAt(write, notelength);
//    } else if (portamento) {
//      seq.setNoteAt(write, data, portaval, false, true, notelength);
//    } else {
//      seq.setNoteAt(write, data, new_instrument, tied, false, notelength);
//    }
//    write += notelength;
//    ++seqAddr;
//    tied = nextTied;
//  }
//  return true;
//}

//bool STHubbardRipper::convertInstruments() {
//  unsigned char* memory = m_sid.m_Memory.data();
//  int instrAddress = m_InstrumentAddress;
//  for (int i = 0; i < 32; ++i) {
//    SteinTronic::byte* ins = m_app.song().getInstrumentAddress(i);
//    if (!m_sid.isLegalAddress(instrAddress))
//      break;
//    int pulsewidth = (memory[instrAddress] + memory[instrAddress + 1] * 256) >> 5;
//    int waveform = memory[instrAddress + 2];
//    int AD = memory[instrAddress + 3];
//    int SR = memory[instrAddress + 4];
//    int unknown = memory[instrAddress + 5];
//    int pulselevel = memory[instrAddress + 6] >> 3;
//    int fx = memory[instrAddress + 7];
//    int pulsedepth = 32; // ?
//    int delay = 0;
//    int waveoff = waveform & 0xfe;
//
//    if ((fx & 1) == 1) { // Drums
//        // If statement: Commando song has no porta for these instruments
//        if (i != 1 && i != 7 && i != 4 && i != 5) {
//            //[fxPorta setObject:[NSNumber numberWithInt:-127] forKey:[NSNumber numberWithInt:i]];
//        }
//            
//        if (fx == 1) { // Ren puka!
//          int hepp = 0;
//          waveform = 0xc1;
//          ins[IOffs::VibLevel] = 0x10;
//          ins[IOffs::VibDepth] = 0xbf;
//            //snd.useWaveTable = TRUE;
//            //waveTable *arp = snd.theWaveTable;
//            //
//            //waveTableEntry *entry = [arp getTableEntry:0];
//            //entry.waveType = snd.waveType;
//            //entry.gate = TRUE;
//            //entry.ringmod = snd.ringmod;
//            //entry.synch = snd.synch;
//            //entry.relativeNote = TRUE;
//            //entry.noteAbs = 0;
//            //entry.noteRel = 0;
//            //
//            //entry = [arp getTableEntry:1];
//            //entry.waveType = WAVEFORM_NOISE;
//            //entry.gate = FALSE;
//            //entry.ringmod = FALSE;
//            //entry.synch = FALSE;
//            //entry.relativeNote = TRUE;
//            //entry.noteAbs = 0;
//            //entry.noteRel = 0;
//            //
//            //entry = [arp getTableEntry:2];
//            //entry.waveType = snd.waveType;
//            //entry.gate = FALSE;
//            //entry.ringmod = snd.ringmod;
//            //entry.synch = snd.synch;
//            //entry.relativeNote = TRUE;
//            //entry.noteAbs = 0;
//            //entry.noteRel = 0;
//            //
//            //arp.restartPos = 2;
//        }
//    }
//    else if ((fx & 2) == 2) { // Sky dive
////      int hipp = 0;
//      ins[IOffs::VibLevel] = 0x10;
//      ins[IOffs::VibDepth] = 0xbf;
//        //// If statement: Commando song has no porta for these instruments
//        //if (i != 2 && i != 8 && i != 5 && i != 6) {
//        //    [fxPorta setObject:[NSNumber numberWithInt:skyDive] forKey:[NSNumber numberWithInt:i]];
//        //}
//    }
//        
//    if ((fx & 4) == 4) { // Oct arp
////      int hopp = 0;
//      m_app.song().m_InstrumentBank.m_ChordData[0] = 0x0c;
//      m_app.song().m_InstrumentBank.m_ChordData[1] = 0x00;
//      m_app.song().m_InstrumentBank.m_ChordData[2] = 0x81;
//      ins[IOffs::FX] = 0x40;
//            //snd.useWaveTable = TRUE;
//            //waveTable *arp = snd.theWaveTable;
//            //
//            //waveTableEntry *entry = [arp getTableEntry:0];
//            //entry.waveType = snd.waveType;
//            //entry.gate = TRUE;
//            //entry.ringmod = snd.ringmod;
//            //entry.synch = snd.synch;
//            //entry.relativeNote = TRUE;
//            //entry.noteAbs = 0;
//            //entry.noteRel = 0;
//            //
//            //entry = [arp getTableEntry:1];
//            //entry.waveType = snd.waveType;
//            //entry.gate = TRUE;
//            //entry.ringmod = snd.ringmod;
//            //entry.synch = snd.synch;
//            //entry.relativeNote = TRUE;
//            //entry.noteAbs = 0;
//            //entry.noteRel = 12;
//            
//            //[arp deleteTableEntry:2];
//
//            //arp.restartPos = 0;
//    }
//    
//    ins[IOffs::GateOnWaveForm] = waveform & 0xff;
//    ins[IOffs::AD] = AD & 0xff;
//    ins[IOffs::SR] = SR & 0xff;
//    ins[IOffs::PulseWidth] = pulsewidth & 0xff;
//    ins[IOffs::PulseDepth] = pulsedepth & 0xff;
//    ins[IOffs::PulseLevel] = pulselevel & 0xff;
//    ins[IOffs::GateOffDelay] = delay;
//    ins[IOffs::GateOffWaveForm] = waveoff;
//    ins[IOffs::RestartGateOff] = 0x80;
//
//    instrAddress += 8;
//  }
//  return true;
//}
