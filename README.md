# proyect_sistemas
Proyecto realizado por: Marta Guisado Iglesias y Marcos Villaverde González.
Curso: INSO 2ºA


Este proyecto implementa un sistema de archivos simulado, utilizando un archivo binario como partición para realizar operaciones básicas de gestión de archivos. Entre las
funcionalidades disponibles se incluyen listar archivos, renombrarlos, eliminarlos, copiarlos, y mostrar su contenido. Todo esto se maneja a través de una interfaz 
interactiva en la línea de comandos.

El sistema de archivos está diseñado en base a una estructura que incluye un superbloque, mapas de bits, inodos, un directorio y bloques de datos. Estas partes trabajan
juntas para permitir una simulación realista y eficiente de un sistema de archivos básico.

- Comandos Disponibles
    El programa tiene los siguientes comándos disponibles para su uso:
        . info: Muestra información sobre el superbloque.
        . bytemaps: Muestra el estado de los mapas de bits de bloques e inodos.
        . dir: Lista los archivos en el sistema de archivos.
        . rename <nombre_antiguo> <nombre_nuevo>: Cambia el nombre de un archivo existente.
        . imprimir <nombre_archivo>: Muestra el contenido de un archivo.
        . remove <nombre_archivo>: Elimina un archivo del sistema.
        . copy <nombre_origen> <nombre_destino>: Copia un archivo dentro del sistema.
        . salir: Termina la ejecución del programa.

- El código está dividido en las siguientes secciones principales:
    . Funciones de utilidad: Operaciones sobre el superbloque, bytemaps, directorio e inodos.
    . Gestión de comandos: Interpreta los comandos del usuario y llama a las funciones correspondientes.
    . Funciones específicas: Implementan la lógica para cada operación (renombrar, imprimir, borrar, copiar).
