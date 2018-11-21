#ifndef STHUBBARDRIPPER_H_INCLUDED
#define STHUBBARDRIPPER_H_INCLUDED

#include "STSIDRipper.h"

#include <vector>

class STHubbardRipper {
public:
  struct SongData {
    int                 m_SongAddress;
    int                 m_TrackAddress[3];
  };

  struct PlayerInstance {
    std::vector<SongData> m_Songs;
    int                   m_InstrumentAddress;
    int                   m_SeqLoAddress;
    int                   m_SeqHiAddress;
    int                   m_SequencesUsedGuesstimate;
    int                   m_FrqAddress;
  };

                        STHubbardRipper(STSIDRipper& sid);
  bool                  scanForData(); 

  std::vector<PlayerInstance>
                        m_Instances;
  STSIDRipper&          m_sid;
};

#endif
