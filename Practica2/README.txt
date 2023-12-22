

Data: 21/12/23




Mejoras:
En relación a las mejoras implementadas, se ha incorporado un control de errores en la sintaxis de los métodos 
internal_bg() y internal_fg(), específicamente en el parámetro args[1]. 
Este ajuste se realizó tras observar que los comandos fg y bg seguidos de un número (por ejemplo, fg\bg 2) arrojaban 
un error en el entorno Bash pero no en nuestra miniShell.

Además, se ha trabajado en la mejora de la legibilidad y organización del código, siguiendo prácticas de programación limpia. 
Para la documentación del código, nos hemos guiado por las pautas proporcionadas en la página
 web (https://www.linkedin.com/pulse/doxygen-santosh-s-malagi/). 
Asimismo, se ha tenido cuidado de no exceder el límite establecido para los comentarios, manteniendo la concisión.

Inicialmente, se consideró la implementación de una función adicional destinada a facilitar la navegación entre comandos anteriores. 
Sin embargo, debido a restricciones de tiempo, esta funcionalidad no ha sido desarrollada en la presente iteración.
 Se deja abierta la posibilidad de considerar su implementación en futuras iteraciones cercanas.


Restricciones
