import sys
import math
from PIL import Image

# --- UMBRALES DE ACEPTACIÓN ---
# 1. La diferencia máxima en un pixel debe ser < 150
UMBRAL_MAX_DIF_PIXEL = 150
# 2. El error cuadrático medio (RMSE) debe ser < 10
UMBRAL_RMSE = 10
# ---------------------------------

def validar_imagenes(ruta_img1, ruta_img2):
    """
    Compara dos imágenes usando los umbrales especificados:
    1. Diferencia Máxima de Píxel (MPD)
    2. Error Cuadrático Medio (RMSE, según la descripción)
    
    Devuelve un tuple con:
    (es_aceptable, mpd_calculado, rmse_calculado, condicion_1_ok, condicion_2_ok)
    """
    try:
        # Abrir imágenes y convertirlas a RGB
        img1 = Image.open(ruta_img1).convert('RGB')
        img2 = Image.open(ruta_img2).convert('RGB')
    except FileNotFoundError as e:
        print(f"Error: No se pudo encontrar el archivo: {e.filename}")
        return None
    except Exception as e:
        print(f"Error al abrir las imágenes: {e}")
        return None

    # Estandarización: Redimensionar la segunda imagen al tamaño de la primera
    if img1.size != img2.size:
        print(f"Advertencia: Las imágenes tienen tamaños diferentes. "
              f"Redimensionando '{ruta_img2}' de {img2.size} a {img1.size} para comparar.")
        img2 = img2.resize(img1.size)

    pixels1 = img1.load()
    pixels2 = img2.load()

    ancho, alto = img1.size
    total_pixeles = ancho * alto

    # Si la imagen está vacía, se considera aceptable
    if total_pixeles == 0:
        return (True, 0.0, 0.0, True, True)

    max_diferencia_pixel = 0.0
    suma_cuadratica_total = 0.0

    # Recorrer todos los píxeles
    for x in range(ancho):
        for y in range(alto):
            r1, g1, b1 = pixels1[x, y]
            r2, g2, b2 = pixels2[x, y]

            # --- Cálculo de la diferencia del píxel (según fórmula) ---
            # Nota: Asumo que "abs(g1,g2)" en tu descripción era un error
            # tipográfico y querías decir "abs(g1-g2)".
            dif_pixel = (abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2)) / 3.0

            # --- Condición 1: Diferencia Máxima ---
            # Se actualiza el valor máximo encontrado hasta ahora
            if dif_pixel > max_diferencia_pixel:
                max_diferencia_pixel = dif_pixel

            # --- Condición 2: Error Cuadrático Medio ---
            # 1. Se calcula el cuadrado de la diferencia
            dif_cuadrado = dif_pixel * dif_pixel
            # 2. Se suman todos los valores cuadráticos
            suma_cuadratica_total += dif_cuadrado

    # --- Calcular resultados finales ---

    # Valor final para la Condición 1:
    mpd_calculado = max_diferencia_pixel

    # Valor final para la Condición 2:
    # 2. (continuación) Se divide la suma por el número de píxeles (media)
    media_cuadratica = suma_cuadratica_total / total_pixeles
    # 3. Se calcula la raíz cuadrada (esto es técnicamente RMSE)
    rmse_calculado = math.sqrt(media_cuadratica)

    # --- Comprobar umbrales ---
    condicion_1_ok = (mpd_calculado < UMBRAL_MAX_DIF_PIXEL)
    condicion_2_ok = (rmse_calculado < UMBRAL_RMSE)
    
    # El resultado es aceptable SÓLO SI ambas condiciones se cumplen
    es_aceptable = condicion_1_ok and condicion_2_ok

    return (es_aceptable, mpd_calculado, rmse_calculado, condicion_1_ok, condicion_2_ok)

# --- Bloque principal para ejecutar el script ---
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("\nError: Número incorrecto de argumentos.")
        print("Uso: python comparar.py <ruta_a_la_imagen_1> <ruta_a_la_imagen_2>")
        sys.exit(1)

    imagen_path_1 = sys.argv[1]
    imagen_path_2 = sys.argv[2]

    # Calcular la validación
    resultados = validar_imagenes(imagen_path_1, imagen_path_2)

    if resultados:
        # Desempaquetar los resultados
        es_aceptable, mpd, rmse, c1_ok, c2_ok = resultados

        print("\n--- INFORME DE VALIDACIÓN DE IMÁGENES ---")
        
        # Reporte Condición 1
        print(f"\n1. Condición: Diferencia Máxima de Píxel (MPD)")
        print(f"   - Umbral:     < {UMBRAL_MAX_DIF_PIXEL}")
        print(f"   - Calculado:  {mpd:.4f}")
        print(f"   - Resultado:  {'CUMPLE' if c1_ok else 'NO CUMPLE'}")

        # Reporte Condición 2
        print(f"\n2. Condición: Error Cuadrático Medio (RMSE)")
        print(f"   - Umbral:     < {UMBRAL_RMSE}")
        print(f"   - Calculado:  {rmse:.4f}")
        print(f"   - Resultado:  {'CUMPLE' if c2_ok else 'NO CUMPLE'}")

        # Conclusión Final
        print("\n-------------------------------------------")
        if es_aceptable:
            print("VEREDICTO FINAL: ACEPTABLE (Ambas condiciones se cumplen)")
        else:
            print("VEREDICTO FINAL: NO ACEPTABLE (Al menos una condición ha fallado)")
        print("-------------------------------------------")