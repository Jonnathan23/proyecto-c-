#include <iostream>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkNiftiImageIOFactory.h> // Para registrar la fábrica NIfTI
#include <string>

#include "DirectionImages.h"
#include "Volumetrics.h"

using namespace std;
using namespace itk;

int main() {
    
    const string brats000 = DirectionImages0::brats0Flair;
    cout << "Mostrar Ruta: " << brats000 << endl;

    Volumetrics volumetrics;
    const bool result = volumetrics.loadVolumetric(brats000);

    if (!result) {
        cout << "Error al cargar el volumen" << endl;        
    }

    cout<<"Volumen cargado correctamente :)"<<endl;
    return 0;
}