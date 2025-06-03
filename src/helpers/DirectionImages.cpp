#include "helpers/DirectionImages.h"

using namespace std;

//? |----------| | Brats0 | |----------|
const string DirectionImages0::brats0Flair =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00000/BraTS2021_00000_flair.nii.gz";

const string DirectionImages0::brats0T1 =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00000/BraTS2021_00000_t1.nii.gz";

const string DirectionImages0::brats0T1c =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00000/BraTS2021_00000_t1ce.nii.gz";

const string DirectionImages0::brats0T2 =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00000/BraTS2021_00000_t2.nii.gz";

const string DirectionImages0::brats0Seg =
    "/media/visionups/JONNA_USB/BRaTS2021_Training_Data/BraTS2021_00000/BraTS2021_00000_seg.nii.gz";


//? |----------| | Brats2 | |----------|
const string DirectionImages2::brats2Flair =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00002/BraTS2021_00002_flair.nii.gz";

const string DirectionImages2::brats2T1 =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00002/BraTS2021_00002_t1.nii.gz";

const string DirectionImages2::brats2T1c =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00002/BraTS2021_00002_t1ce.nii.gz";

const string DirectionImages2::brats2T2 =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00002/BraTS2021_00002_t2.nii.gz";

const string DirectionImages2::brats2Seg =
    "/media/visionups/JONNA_USB/BRaTS2021_Training_Data/BraTS2021_00002/BraTS2021_00002_seg.nii.gz";

    
//? |----------| | Brats3 | |----------|
const string DirectionImages3::brats3Flair =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00003/BraTS2021_00003_flair.nii.gz";

const string DirectionImages3::brats3T1 =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00003/BraTS2021_00003_t1.nii.gz";

const string DirectionImages3::brats3T1c =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00003/BraTS2021_00003_t1ce.nii.gz";

const string DirectionImages3::brats3T2 =
    "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00003/BraTS2021_00003_t2.nii.gz";

const string DirectionImages3::brats3Seg =
    "/media/visionups/JONNA_USB/BRaTS2021_Training_Data/BraTS2021_00003/BraTS2021_00003_seg.nii.gz";

//TODO const std::map<std::string, BratsPaths> allBratsMap

//? |----------| | AllBratsMap | |----------|
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
