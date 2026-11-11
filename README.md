# Proyecto 3 - Space Travel

Proyecto final de Gráficas por Computadora desarrollado en C++ utilizando SDL2.

El proyecto implementa un pequeño sistema solar renderizado mediante un pipeline 3D por software. SDL2 se utiliza para la creación de la ventana, entrada de teclado y presentación final del framebuffer.

## Características principales

- Rasterización de triángulos por software.
- Framebuffer propio.
- Z-buffer para prueba de profundidad.
- Transformaciones Model, View y Projection.
- Cámara 3D en tercera persona.
- Nave espacial modelada en Blender y cargada desde un archivo OBJ.
- Movimiento libre de la nave.
- Aceleración y frenado progresivos.
- Movimiento vertical.
- Colisiones con cuerpos celestes.
- Sol con shader animado.
- Planeta rocoso.
- Planeta verde.
- Planeta con anillos.
- Luna orbitando un planeta.
- Órbitas calculadas utilizando tiempo transcurrido.
- Rotación axial de los cuerpos utilizando tiempo transcurrido.
- Skybox procedural con estrellas.
- Efecto de warp animado.
- Cambio dinámico del campo de visión durante warp.
- Estelas de estrellas durante warp.
- Geometría procedural para esferas, cubos y anillos.
- Base visible al aproximarse al planeta verde.
- Diferentes fragment shaders ejecutados en CPU.
- Rasterizador optimizado utilizando evaluación incremental de funciones de borde.

## Sistema solar

La escena contiene los siguientes elementos:

1. Sol.
2. Planeta rocoso.
3. Planeta verde.
4. Planeta con anillos.
5. Luna.

Los planetas orbitan alrededor del Sol utilizando el tiempo transcurrido de ejecución.

También poseen rotación axial independiente basada en tiempo, por lo que la velocidad de las animaciones no depende de los FPS.

La Luna orbita alrededor del planeta verde.

## Planeta verde

Al acercarse al planeta verde aparece una pequeña base sobre su superficie.

La base incluye:

- Plataforma.
- Edificio.
- Torre.
- Baliza.

Las estructuras permanecen asociadas a la posición y rotación del planeta durante su movimiento orbital.

## Nave espacial

La nave fue modelada utilizando Blender y exportada en formato OBJ.

El archivo utilizado por el programa se encuentra en:

`models/nave.obj`

El programa implementa un cargador OBJ propio para convertir el modelo en triángulos que posteriormente son procesados por el pipeline de software.

## Controles

| Tecla | Acción |
|---|---|
| W | Acelerar |
| S | Frenar / reversa |
| A | Girar a la izquierda |
| D | Girar a la derecha |
| Q | Subir |
| E | Bajar |
| SHIFT | Activar Warp |
| ESC | Salir |

## Warp

Al mantener presionada la tecla `SHIFT` se activa el modo Warp.

El efecto incluye:

- Aumento progresivo de velocidad.
- Estrellas convertidas en estelas.
- Cambio dinámico del campo de visión.
- Cambio ligero del color del fondo.
- Reducción de capacidad de giro durante alta velocidad.
- Salida progresiva del efecto al soltar la tecla.

El efecto es animado mediante una variable que cambia gradualmente entre el estado normal y el estado de warp.

## Skybox

Las estrellas se generan proceduralmente alrededor de la cámara utilizando direcciones distribuidas sobre una esfera virtual.

El fondo depende únicamente de la orientación de la cámara y no de su posición.

Por esta razón:

- las estrellas cambian al girar;
- no se acercan al desplazarse;
- se comportan como elementos a distancia infinita.

## Pipeline gráfico

El proyecto utiliza un pipeline gráfico implementado por software.

El flujo principal es:

`Modelo 3D -> Model Matrix -> View Matrix -> Projection Matrix -> Rasterización -> Fragment Shader -> Z-buffer -> Framebuffer -> SDL2`

SDL2 no se utiliza para renderizado 3D.

La biblioteca se utiliza únicamente para:

- crear la ventana;
- recibir eventos de teclado;
- mostrar el framebuffer final.

## Fragment shaders

El proyecto implementa diferentes shaders calculados en CPU.

### Nave

Utiliza iluminación y diferentes tonos azules dependiendo de la orientación de cada superficie.

### Sol

Utiliza un shader animado que modifica su patrón con respecto al tiempo transcurrido.

### Planeta rocoso

Genera proceduralmente regiones de distintos tonos de roca.

### Planeta verde

Genera regiones verdes y azuladas para distinguir zonas similares a tierra y agua.

### Anillos

Utiliza diferentes bandas de color según la posición de cada fragmento.

## Z-buffer

Cada píxel posee un valor de profundidad.

Antes de escribir un nuevo fragmento en el framebuffer se compara su profundidad contra el valor almacenado en el Z-buffer.

Esto permite determinar correctamente qué superficie se encuentra delante de otra.

## Rasterización

Los modelos se convierten en triángulos.

Cada triángulo:

1. Se transforma al espacio de pantalla.
2. Obtiene un bounding box.
3. Se recorren los píxeles candidatos.
4. Se calculan coordenadas baricéntricas.
5. Se interpola la profundidad.
6. Se ejecuta el fragment shader.
7. Se realiza la prueba de profundidad.
8. Se escribe el resultado en el framebuffer.

El rasterizador utiliza funciones de borde incrementales para reducir cálculos repetidos durante el recorrido de píxeles.

## Geometría procedural

Además del modelo OBJ de la nave, varios objetos se generan directamente mediante código antes de ser rasterizados.

Se generan proceduralmente:

- Esferas.
- Cubos.
- Anillos.

Las esferas se utilizan para los planetas, el Sol y la Luna.

Los cubos se utilizan para las estructuras de la base.

El anillo se utiliza para el planeta anillado.

## Colisiones

Se implementaron colisiones básicas entre la nave y los siguientes objetos:

- Sol.
- Planeta rocoso.
- Planeta verde.
- Planeta anillado.
- Luna.

La cámara también es corregida para evitar que atraviese los cuerpos celestes.

## Compilación

El proyecto fue desarrollado y probado utilizando MSYS2 UCRT64 con GCC y SDL2.

### Dependencias

Se requiere:

- C++17
- SDL2
- MSYS2 UCRT64
- GCC / g++

### Compilar

Desde MSYS2 UCRT64:

```bash
cd /c/Users/rober/Documents/proyecto_3_space_travel

g++ src/main.cpp -std=c++17 $(sdl2-config --cflags --libs) -O2 -o build/space_travel.exe
```

### Ejecutar

```bash
./build/space_travel.exe
```

## Estructura del proyecto

```text
proyecto_3_space_travel/
│
├── models/
│   ├── nave.obj
│   └── nave.mtl
│
├── src/
│   ├── camera.h
│   ├── color.h
│   ├── framebuffer.h
│   ├── geometry.h
│   ├── main.cpp
│   ├── math3d.h
│   ├── mesh.h
│   ├── obj_loader.h
│   ├── scene.h
│   ├── shaders.h
│   └── triangle.h
│
├── build/
│
├── captures/
│
├── .gitignore
└── README.md
```

## Video de demostración

https://youtu.be/r0csmtpH7HI

## Uso de inteligencia artificial

Durante el desarrollo del proyecto se utilizó inteligencia artificial como herramienta de apoyo para:

- explicar conceptos de gráficas por computadora;
- revisar errores de compilación;
- proponer estructuras de código;
- apoyar en depuración y optimización;
- documentar el funcionamiento del proyecto.

El código fue integrado, probado y ajustado durante el desarrollo del proyecto.

## Autor

José Roberto Rodríguez Reyes

Universidad del Valle de Guatemala

Ciencias de la Computación y Tecnologías de la Información