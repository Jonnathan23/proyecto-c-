#!/usr/bin/env python3
# generar_estadisticas.py

import sys
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

def main():
    if len(sys.argv) < 3:
        print("Uso: generar_estadisticas.py <ruta_csv> <dir_salida>")
        sys.exit(1)

    csv_path = sys.argv[1]
    out_dir  = sys.argv[2]

    # Leer valores (se asume un CSV de una sola columna sin cabecera)
    data = pd.read_csv(csv_path, header=None, names=['valor'])

    # Estadísticos básicos
    minimo = data['valor'].min()
    maximo = data['valor'].max()
    media  = data['valor'].mean()
    q1     = data['valor'].quantile(0.25)
    mediana= data['valor'].quantile(0.50)
    q3     = data['valor'].quantile(0.75)
    iqr    = q3 - q1

    # Detectar outliers (1.5 * IQR)
    lim_inf = q1 - 1.5 * iqr
    lim_sup = q3 + 1.5 * iqr
    outliers = data[(data['valor'] < lim_inf) | (data['valor'] > lim_sup)]

    # Guardar un reporte de texto
    reporte_txt = os.path.join(out_dir, "reporte_estadisticas.txt")
    with open(reporte_txt, "w") as f:
        f.write(f"Minimo: {minimo:.2f}\n")
        f.write(f"Q1 (25%): {q1:.2f}\n")
        f.write(f"Mediana (50%): {mediana:.2f}\n")
        f.write(f"Q3 (75%): {q3:.2f}\n")
        f.write(f"Maximo: {maximo:.2f}\n")
        f.write(f"Media: {media:.2f}\n")
        f.write(f"IQR: {iqr:.2f}\n")
        f.write(f"Outliers (1.5×IQR): {len(outliers)}\n")

    # 1) Boxplot
    plt.figure(figsize=(4,6))
    plt.boxplot(data['valor'], vert=True, patch_artist=True,
                boxprops=dict(facecolor='lightblue', color='blue'),
                medianprops=dict(color='red'),
                whiskerprops=dict(color='black'),
                capprops=dict(color='black'),
                flierprops=dict(marker='o', markerfacecolor='red', markersize=3, linestyle='none'))
    plt.title("Diagrama de Caja y Bigotes")
    plt.ylabel("Intensidad píxel")
    boxplot_png = os.path.join(out_dir, "boxplot.png")
    plt.savefig(boxplot_png)
    plt.close()

    # 2) Histograma
    plt.figure(figsize=(6,4))
    plt.hist(data['valor'], bins=30, color='gray', edgecolor='black')
    plt.title("Histograma de Intensidades")
    plt.xlabel("Intensidad píxel")
    plt.ylabel("Frecuencia")
    histo_png = os.path.join(out_dir, "histograma.png")
    plt.savefig(histo_png)
    plt.close()

    # 3) Gráfico de barras de estadísticos básicos
    plt.figure(figsize=(6,4))
    categories = ['Mín', 'Q1', 'Med', 'Q3', 'Máx']
    values = [minimo, q1, mediana, q3, maximo]
    plt.bar(categories, values, color='skyblue', edgecolor='black')
    plt.title("Resumen de Estadísticos Básicos")
    plt.ylabel("Valor")
    barras_png = os.path.join(out_dir, "estadisticos_basicos.png")
    plt.savefig(barras_png)
    plt.close()

    # Otras gráficas que desees…  

    print("OK")  # Mensaje de éxito

if __name__ == "__main__":
    main()
