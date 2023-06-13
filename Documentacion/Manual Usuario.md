# **MANUAL DE USUARIO - PRACTICA 1**

  

**

GRUPO 21

  

## Descripcion

  

**

Como primera actividad del laboratorio se solicita que se desarrolle un sistema interactivo simulado en donde se apliquen los conocimientos acerca del microcontrolador Arduino adquiridos hasta el momento. Este sistema consiste en términos de hardware, el cuál es simulado, en un par de matrices de LEDs que servirán como principal medio de salida. Algunos botones y dos potenciómetros cuya función será la de fungir como medios de entrada al sistema. El corazón de todo esto será el microcontrolador Arduino que orquestrará el funcionamiento de los demás dispositivos. Del lado del software se solicita la implementanción de un juego simple, que será descrito posteriormente, junto con la interfaz que permita acceder al juego; así como también a algunas funcionalidades que acompañarán al juego.

  

**

  

## Diseño

  

**

El juego y la interfaz que lo acompaña usarán los botones disponibles para ser manipulados. La Figura 1 muestra el arreglo sugerido para dichos elementos. Como medio de salida se tendrán dos matrices de LEDs cuyas dimensiones deben ser de 8x8. Estas deben ser integradas para que funjan como una sola matriz. Una de éstas utilizará un driver para su operación mientras que la otra se deberá implementar sin nada más que las conexiones del microcontrolador. El sistema será manejado por una serie de menús que permitarán al usuario acceder a cada una de las funcionalidades solicitadas.
![](!%5B%5D%28https://github.com/rubenralda/ACE1-23VJ0778G21PRA1/blob/master/Documentacion/img/Dise%C3%B1o.png?raw=true%29)
  



**

  

## Funcionamiento del Juego

  

**

**INICIO:**

Al iniciar el juego se muestra un mensaje que muestra los siguientes datos:

pRA1-vJ-2023< gRUPO XXl >


  ![](https://github.com/rubenralda/ACE1-23VJ0778G21PRA1/blob/master/Documentacion/img/msj.png?raw=true)
  

**Cambio direccion Mensaje:**

La dirección del mensaje podrá ser cambiada mientras este se muestra al presionar el botón “D”. El botón “K” servirá para poder salir del mensaje hacia el menú principal cuado este es presionado por más de 2 segundos. La velocidad del mensaje podrá ser controlada por un mecanismo que se describirá más adelante.

  
**Menu :**
Al menu se puede ingresar despues de presionar el boton "k" durante 3 segundos cuando este mostrandose el mensaje inicial.
Luego de salir del mensaje inicial se mostrará un menú principal desde donde se podrá acceder a tres secciones: Juego, Estadísticas y Configuración. El diseño del menú solicitado . La selección de opciones es simple. Con el botón de dirección hacia la izquierda seleccionará la opción de Juego y provocará que el sistema inicie una nueva partida. Utilizando el botón “D” en esta parte provocará que se comiencen a mostrar las estadísticas que se describirán más adelante. Y, por último, el botón de dirección hacia la derecha permitirá acceder al menú de configuración que también se describira en secciones posteriores de este documento.
![](https://github.com/rubenralda/ACE1-23VJ0778G21PRA1/blob/master/Documentacion/img/Menu.png?raw=true)

**Como iniciar el juego:**

El juego iniciara en el momento en que el usuario este en el Menu, se debera presionar el boton "izquierda" y podra acceder al juego.



**Como jugar:**

El juego a desarrollar consiste en un pequeño avión cuya meta es destruir una serie de objetivos que se presentarán. En la Figura 2 se mostrará el arreglo inicial del juego. El avión destruirá los objetivos lanzando un proyectil que descenderá poco a poco hasta destruir un objetivo o chocar con el suelo en el caso de que se falle el tiro.

Una vez dentro del juego, el botón “D” tendrá la función de disparar el proyectil. Los botones de dirección cambiarán la dirección del avión, mientras que el botón restante, botón “K”, permitirá acceder al menú de pausa que se describirá más adelante.
  ![](https://github.com/rubenralda/ACE1-23VJ0778G21PRA1/blob/master/Documentacion/img/juego.png?raw=true)

**

  

**Pausa y configuracion**


Durante una partida en curso es posible acceder a un menú de pausa. En este menú solamente será necesario que se muestre el número de vidas que le quedan al jugador. Para volver al juego desde este menú se deberá presionar el botón “K” por 2 segundos. Para retornar al menú principal también se deberá presionar el botón “K”, pero para este caso serán 3 segundos los que debe permanecer presionado.

El menú de configuración será la parte donde se permitirá modificar parámetros escenciales del funcionamiento del sistema y del juego principalmente. En esta parte se mostrarán dos barras horizontales cuya longitud dependerá del valor del parámetro que corresponda. Una de las barras estará en la parte superior de las matrices de LEDs, ésta reflejará el valor actual de la Velocidad del juego y del mensaje inicial. La otra barra se ubicará en la parte inferior y permitirá observar el valor de las Vidas iniciales que tendrá el jugador al iniciar una partida.

  ## Estadísticas

  

### Ingreso

  

A partir del menú del juego, presionando el botón “D” en esta parte, provocará que se comiencen a mostrar las estadísticas.

  

### Gráfica

  

En este apartado se despliega una gráfica de barras, la cuál representa el porcentaje respectivo de los últimos 5 puntajes de partidas almacenadas en memoria.

Al mencionar porcentaje, se refiere a que es una gráfica del puntaje obtenido en la partida a razón de la sumatoria de todos los cinco puntajes almacenados en memorias, dicha razón es el porcentaje que se ve desplegado o trazado en cada gráfica.

  

![stats](/Documentacion/img/stats.png)

  

### Salida

  

Para salir del menú de las estadísticas hacia el menú principal se deberá presionar el botón “K” por tres segundos.

**
