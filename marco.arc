//pf2024 Ejem1

declaracion
	/*Comentario*/
	entero posicion, a,b,c,f;
	cadena d1;
	entero d;
	caracter g = 'g';
	booleano e;
	//entero CONSTANTE_1;
	//funcion entero sumar ( entero x, entero y );
fin_declaracion

principal
	a = 5;
	b =3;
	c = 2;
	/* Encabezado, deberá llevar el nombre del lenguaje*/
	/*Pal_Reserv Declaración de variables*/
	/*Cuerpo del programa, inicio etc*/
	imprimir "Dame un numero, Hola mundo";
	//var = leer_entero;
	e = FALSO;
	/* solicita un número */
	/* Guarda un valor entero en una variable */
	d = a*b-c;
	d = a/(b-c);
	imprimir d;

	si(VERDADERO){
		imprimir a;
		imprimir "CUERPO CONDICIONAL\n";
	} si_no {
		imprimir g;
	}
fin_principal

funcion entero sumar(entero x, entero y) {
	entero resultado = x + y;
	imprimir "el resultado es: " + resultado;
	retornar resultado + 100.01;
} // Esto es un comentario

/*
COMENTARIO DE BLOQUE
*/