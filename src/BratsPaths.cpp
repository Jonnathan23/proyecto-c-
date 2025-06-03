#include "BratsPaths.h"
#include "helpers/DirectionImages.h"

const std::map<std::string, BratsPaths> allBratsMap = {
    { "brats0",
      BratsPaths{
          DirectionImages0::brats0Flair,
          DirectionImages0::brats0T1,
          DirectionImages0::brats0T1c,
          DirectionImages0::brats0T2,
          DirectionImages0::brats0Seg
      }
    },
    { "brats2",
      BratsPaths{
          DirectionImages2::brats2Flair,
          DirectionImages2::brats2T1,
          DirectionImages2::brats2T1c,
          DirectionImages2::brats2T2,
          DirectionImages2::brats2Seg
      }
    },
    { "brats3",
      BratsPaths{
          DirectionImages3::brats3Flair,
          DirectionImages3::brats3T1,
          DirectionImages3::brats3T1c,
          DirectionImages3::brats3T2,
          DirectionImages3::brats3Seg
      }
    }
};
