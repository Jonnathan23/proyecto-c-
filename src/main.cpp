#include <iostream>
#include <string>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkNiftiImageIOFactory.h>  // Para registrar la fábrica NIfTI

using namespace std;
using namespace itk;

int main() {
    // Registrar la fábrica NIfTI para que itkImageFileReader la encuentre
    NiftiImageIOFactory::RegisterOneFactory();

    // Ruta al archivo .nii.gz
    string filePath = "/media/visionups/JONNA_USB/BraTS2021_Training_Data/BraTS2021_00000/BraTS2021_00000_flair.nii.gz";

    // Definir el tipo de imagen (float 3D)
    using ImageType = Image<float, 3>;

    // Crear el lector
    ImageFileReader<ImageType>::Pointer reader = ImageFileReader<ImageType>::New();
    reader->SetFileName(filePath);

    try {
        reader->Update();
    } catch (ExceptionObject &e) {
        cerr << "Error al leer el volumen: " << e << endl;
        return EXIT_FAILURE;
    }

    // Obtener la imagen cargada
    ImageType::Pointer volume = reader->GetOutput();

    // Obtener el tamaño (dimensiones) del volumen
    ImageType::RegionType region = volume->GetLargestPossibleRegion();
    ImageType::SizeType size = region.GetSize();

    cout << "Volumen leído correctamente." << endl;
    cout << "Dimensiones del volumen (x, y, z): "
         << size[0] << " x "
         << size[1] << " x "
         << size[2] << endl;

    return EXIT_SUCCESS;
}