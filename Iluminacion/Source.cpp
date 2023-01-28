/*!
	Source.cpp

	Proyecto SGI 

	@author		Rafael Estelles Tatay' <restta@upv.es>
	@date		Nov,2022
 */

#define PROYECTO "PROYECTO"

#include <iostream>			
#include <Utilidades.h>
#include <sstream>
#include <mciapi.h>

using namespace std;

// Globales
const int MAX_HEIGHT = 500; // Alto máximo del terreno
const int MIN_HEIGHT = -1; // Altura mínima del terreno
const int MAX_HEIGHT_DELTA = 1; // Diferencia máxima de altura entre un triángulo y sus vecinos
static float giroxTA = 0, giroyTA = 0;
int xanterior, yanterior;
GLubyte objeto[1];
const int EXTENSION = 10000; //Longitud malla
const int RESOLUCION_TERRENO = 150; //Nº polígonos en cada aldo
int ALTURA_INICIAL = -300; //Altura inicial
const int AMPLITUD_MONTAÑAS = 700;
const int INICIO_AGUA = -400;
const double HELIPUERTO = RESOLUCION_TERRENO ;
static float altura[RESOLUCION_TERRENO + 1][RESOLUCION_TERRENO + 1];
static float velocidad = 0;
const int static tasaFps = 60;
static float modulo = 0;
static float dpX;
static float dpY;
static float dpZ;
static const float MAX_VELOCIDAD = 360;
static const int MAX_ALTURA = 1350;
static const int MIN_ALTURA = -399; //10 más qeu el agua para que no pueda tocar el agua
static float ojo[] = { 0,0, ALTURA_INICIAL*1};
static float amplitudes[4] = { 10, 30, 20, 40 };
static bool verCabina = true; //cabina
//ILUMINACION
bool dia = true;
bool foco = true;
bool noche = false;

//VELOCIMETRO
const float velocidadMin = 0;
const float velocidadMax = 250;
//TEXTURAS
GLuint water;
GLuint grass;
GLuint mountain;
GLuint snow;
GLuint sand;
GLuint heliP;
GLuint cabina;
GLuint velocimetro;
GLuint agujaVelocimetro;
GLuint brujula;
GLuint alturometro;
GLuint altitud;

float startMountain = 10;
float startSnow = AMPLITUD_MONTAÑAS * 0.7;
float startSand = INICIO_AGUA * 0.6;

static bool pulsado = false;
static bool autopilot = false;
static float AutAltura = 0;

//MUSICA
bool musica = true;
LPCWSTR play = L"play musica.mp3 repeat";
LPCWSTR pause = L"pause musica.mp3";
LPCWSTR resume = L"resume musica.mp3";

LPCWSTR explosion = L"play explosion.mp3 ";

double elevacion(double x, double y) {
	// Se definen constantes para el radio del lago y la distancia hasta la base de la montaña
	const double RADIO_LAGO = RESOLUCION_TERRENO/5;
	const double MLAGO = RESOLUCION_TERRENO / 2;
	const double DISTANCIA_MONTAÑA = 10.0;
	const double rangoMontaña = RESOLUCION_TERRENO / 10;
	static float dimension = EXTENSION / RESOLUCION_TERRENO;
	const double angulo = 35;
	// Se calcula la distancia desde el centro del lago hasta el punto (x, y)
	double distancia = sqrt((x - MLAGO)*(x - MLAGO) + (y - MLAGO) * (y - MLAGO));
	if (x <= rangoMontaña && y >= RESOLUCION_TERRENO - rangoMontaña) {
		double aux = rangoMontaña * dimension * tan(angulo * PI / 180);
		return (dimension * (max(x, (y - RESOLUCION_TERRENO + rangoMontaña))) * tan(angulo * PI / 180));
	}
	else if (y <= rangoMontaña && x >= RESOLUCION_TERRENO - rangoMontaña) {
		double aux = rangoMontaña * dimension * tan(angulo * PI / 180);
		return (dimension * (max(y, (x - RESOLUCION_TERRENO + rangoMontaña))) * tan(angulo * PI / 180));
	}
	else if (x <= rangoMontaña || y <= rangoMontaña) {
		double aux = rangoMontaña * dimension * tan(angulo * PI / 180);
		return aux - (dimension *min(x, y) * tan(angulo * PI / 180));
	}
	else if (x >= RESOLUCION_TERRENO - rangoMontaña || y >= RESOLUCION_TERRENO - rangoMontaña) {
		//double aux = rangoMontaña * dimension * tan(35 * PI / 180);
		return (dimension * (max(x, y)-RESOLUCION_TERRENO + rangoMontaña) * tan(angulo * PI / 180));
	}
	
	//if (distancia <= RADIO_LAGO/12) { return MIN_ALTURA + 100; }
	 //Si el punto está dentro del lago, se devuelve la altura en función de la distancia al centro
	if (distancia <= RADIO_LAGO) {
		return max(MIN_ALTURA-20, - 1000 / (1 + distancia) * 3);
	}

	

	// Si el punto está en la montaña, se devuelve la altura de la montaña
	if (distancia <= RADIO_LAGO + rangoMontaña) {
		int frecuencia = 3;
		return AMPLITUD_MONTAÑAS*sin(x/frecuencia)*sin(y/frecuencia);
	}
	// Si el punto está en la zona plana entre el lago y la montaña, se devuelve la altura del suelo plano
	
	return 0.0;
}
void init()
// Inicializaciones
{
	cout << "Iniciando " << PROYECTO << endl;
	cout << "GL version " << glGetString(GL_VERSION) << endl;
	cout << "CONTROL DE ALTURA: Para aumentar pulse \"flecha arriba\", para disminuir pulse \"flecha abajo\"." << endl;
	cout << "MODO DIURNO/NOCTURNO: Alterne con \"L/l\" para alternar entre el modo diurno y nocturno." << endl;
	cout << "GIRO DEL HELICOPTERO: Para rotar a la derecha pulse \"flecha derecha\", para rotar a la izquierda pulse \"flecha izquierda\"." << endl;
	cout << "ACELERAR: Pulse \"A/a\" para aumentar la velocidad." << endl;
	cout << "DESACELERAR: Pulse \"Z/z\" para disminuir la velocidad." << endl;
	cout << "MARCHA ATRAS: Pulse \"R/r\" para aumentar la velocidad marcha atrás cuando el helicoptero este parado y acelere para dejar de ir marcha atrás" << endl;
	cout << "PILOTO AUTOMÁTICO: Alterne con \"Q/q\" para fijar el helicoptero a una altura sobre el suelo." << endl;
	cout << "MOSTRAR CABINA: Alterne con \"C/c\" para activar o desactivar la interfaz de la cabina." << endl;
	cout << "ACTIVAR FOCO: Alterne con \"F/f\" para activar o desactivcar el foco." << endl;
	cout << "MUSICA: Alterne con \"M/m\" para activar o desactivcar la musica." << endl;
	cout << "CABECEO Y GUIÑADA CAMARA: Mantenga el boton izquierdo del ratón para dismvariar el cabeceo y la guiñada." << endl;
	
	//MUSICA
	mciSendString(play, NULL, 0, NULL);

	//Sacamos la altura
	for (int i = 0; i < RESOLUCION_TERRENO; i++) {
		for (int j = 0; j < RESOLUCION_TERRENO; j++) {
			altura[j][i] = elevacion(j, i);
		}
	}

	glEnable(GL_TEXTURE_2D);//Texturas

	glGenTextures(1, &water);
	glBindTexture(GL_TEXTURE_2D, water);
	loadImageFile((char*)"water.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &grass);
	glBindTexture(GL_TEXTURE_2D, grass);
	loadImageFile((char*)"cesped.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//glEndList();
	glGenTextures(1, &mountain);
	glBindTexture(GL_TEXTURE_2D, mountain);
	loadImageFile((char*)"roca.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &snow);
	glBindTexture(GL_TEXTURE_2D, snow);
	loadImageFile((char*)"nieve.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &sand);
	glBindTexture(GL_TEXTURE_2D, sand);
	loadImageFile((char*)"sand.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &heliP);
	glBindTexture(GL_TEXTURE_2D, heliP);
	loadImageFile((char*)"hel.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &cabina);
	glBindTexture(GL_TEXTURE_2D, cabina);
	loadImageFile((char*)"cabina.png");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &velocimetro);
	glBindTexture(GL_TEXTURE_2D, velocimetro);
	loadImageFile((char*)"velocimetro.png");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &agujaVelocimetro);
	glBindTexture(GL_TEXTURE_2D, agujaVelocimetro);
	loadImageFile((char*)"aguja.png");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &brujula);
	glBindTexture(GL_TEXTURE_2D, brujula);
	loadImageFile((char*)"brujula.png");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &alturometro);
	glBindTexture(GL_TEXTURE_2D, alturometro);
	loadImageFile((char*)"alturometro.png");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &altitud);
	glBindTexture(GL_TEXTURE_2D, altitud);
	loadImageFile((char*)"alturometro.png");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Iluminacion
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_FOG);
	GLfloat tono[] = { 0.7, 0.65, 0.75 };
	glFogfv(GL_FOG_COLOR, tono);
	glFogf(GL_FOG_DENSITY, 0.0007);

	//Cabina
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);

	glShadeModel(GL_SMOOTH);

	//AMBIENTE
	const GLfloat F0A[] = { 0.2,0.2,0.2,1 };
	glLightfv(GL_LIGHT0, GL_LIGHT_MODEL_AMBIENT, F0A); //Iambiente
	const GLfloat F0D[] = { 0.2,0.2,0.2,1 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, F0D); //Idifusa
	const GLfloat F0S[] = { 0.2,0.2,0.2,1 };
	glLightfv(GL_LIGHT0, GL_SPECULAR, F0S); //Iespecular

	//FOCO fijo
	const GLfloat F1A[] = { 0.01,0.01,0.01,1 };
	glLightfv(GL_LIGHT1, GL_LIGHT_MODEL_AMBIENT, F1A);
	const GLfloat F1D[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, F1D);
	const GLfloat F1S[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT1, GL_SPECULAR, F1S);

	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 15.0); //radio
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 3.0); //factor de atenuacion

	//FOCO fijo
	const GLfloat F2A[] = { 0.01,0.01,0.01,1 };
	glLightfv(GL_LIGHT2, GL_LIGHT_MODEL_AMBIENT, F1A);
	const GLfloat F2D[] = { 0.9,0.9,0.9,1, 1 };
	glLightfv(GL_LIGHT2, GL_DIFFUSE, F1D);
	const GLfloat F2S[] = { 0.9,0.9,0.9,1, 1 };
	glLightfv(GL_LIGHT2, GL_SPECULAR, F1S);

	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 40.0); //radio
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 10.0); //factor de atenuacion

	glEnable(GL_DEPTH_TEST);
}

void generarTerreno() {
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS); //Texturas
	static float dimension = EXTENSION / RESOLUCION_TERRENO;
	static float media = EXTENSION / 2;
	for (int i = 0; i < RESOLUCION_TERRENO; i++) {

		for (int j = 0; j < RESOLUCION_TERRENO; j++) {

			//TEXTURAS
			if (altura[i + 1][j + 1] < startSand && altura[i][j] < startSand && altura[i][j + 1] < startSand && altura[i + 1][j] < startSand) { //Textura Arena
			glBindTexture(GL_TEXTURE_2D, sand);
			}
			else if ((altura[i + 1][j + 1] <= startMountain && altura[i][j] <= startMountain) && (altura[i][j + 1] <= startMountain && altura[i + 1][j] <= startMountain)) { //Textura Cesped
				glBindTexture(GL_TEXTURE_2D, grass);
			}
			else if (altura[i + 1][j + 1] <= startSnow && altura[i][j] <= startSnow && altura[i][j + 1] <= startSnow && altura[i + 1][j] <= startSnow){ //Textura Roca
				glBindTexture(GL_TEXTURE_2D, mountain);
			}
			else { //textura Nieve
				glBindTexture(GL_TEXTURE_2D, snow);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			if (noche) {
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			}
			else {
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			}

			//Creación
			glBegin(GL_TRIANGLE_STRIP);
			glColor3f(1, 1, 1);
			glTexCoord2f(0, 1); //Texturas
			glVertex3f(dimension * j - media, dimension * (i + 1) - media,altura[j][i+1]);
			glTexCoord2f(0, 0); //Texturas
			glVertex3f(dimension * j - media, dimension * (i)-media, altura[j][i]);
			glTexCoord2f(1, 1); //Texturas
			glVertex3f(dimension * (j + 1) - media, dimension * (i + 1) - media, altura[j + 1][i + 1]);
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
			glColor3f(1, 1, 1);
			glTexCoord2f(0, 0);
			glVertex3f(dimension * j - media, dimension * (i)-media, altura[j][i]);
			glTexCoord2f(1, 1);
			glVertex3f(dimension * (j + 1) - media, dimension * (i + 1) - media, altura[j + 1][i + 1]);
			glTexCoord2f(1, 0);
			glVertex3f(dimension * (j + 1) - media, dimension * (i)-media, altura[j + 1][i]);
			glEnd();

			/*if (altura[j][i] == 0 && altura[j+1][i] == 0 && altura[j + 1][i + 1] && altura[j][i + 1] == 0) {
				glPushMatrix();
				
				glTranslatef(ojo[0]+50, ojo[1]+50, ojo[2]);
				glScalef(100, 100, 100);
				glCallList(arbol);
				glPopMatrix();
			}*/

		}
	}
	glPopAttrib(); //Texturas
	glPopMatrix();
}
void generarAgua() {
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS); //Texturas

	//Texturas
	glBindTexture(GL_TEXTURE_2D, water);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	if (noche) {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

	//Creación
	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(20, 20); //Texturas
	glVertex3f(EXTENSION/2, EXTENSION/2, INICIO_AGUA);
	glTexCoord2f(20, 0); //Texturas
	glVertex3f(EXTENSION/2, -EXTENSION/2, INICIO_AGUA);
	glTexCoord2f(0, 0); //Texturas
	glVertex3f(-EXTENSION/2, -EXTENSION/2, INICIO_AGUA);
	glTexCoord2f(0, 30); //Texturas
	glVertex3f(-EXTENSION/2, EXTENSION/2, INICIO_AGUA);
	glEnd();

	glPopAttrib(); //Texturas
	glPopMatrix();
}
void helipuerto() {
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS); //Texturas
	glBindTexture(GL_TEXTURE_2D, heliP);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	if (noche) {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
	//Creación
	glPushMatrix();
	glTranslatef(0, 50, 0);
	glRotatef(45, 0,0,1);
	glTranslatef(-HELIPUERTO / 4,  -HELIPUERTO/4, MIN_ALTURA + 10);
	glutSolidCylinder(HELIPUERTO , MIN_ALTURA, 4, 4);
	glPopMatrix();

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS); //Texturas

	//Texturas
	glBindTexture(GL_TEXTURE_2D, heliP);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	if (noche) {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
	
	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(0, 0); //Texturas
	glVertex3f(-HELIPUERTO / 2, -HELIPUERTO / 2, MIN_ALTURA + 20);
	glTexCoord2f(1, 0); //Texturas
	glVertex3f(HELIPUERTO / 2, -HELIPUERTO / 2, MIN_ALTURA + 20);
	glTexCoord2f(1, 1); //Texturas
	glVertex3f(HELIPUERTO / 2, HELIPUERTO / 2, MIN_ALTURA+20);
	glTexCoord2f(0, 1); //Texturas
	glVertex3f(-HELIPUERTO / 2, HELIPUERTO / 2, MIN_ALTURA+20);
	glEnd();
	glPopMatrix(); 
	glColor3f(1, 1, 1);


	glPopAttrib(); //Texturas
	glPopMatrix();
}
void crearCabina() {
	if (verCabina) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, cabina);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);

		static float planoS[] = { 0.0, 1.0, 0.0, 0.5 };
		static float planoT[] = { 0.0, 0.0, 1.0, -0.5 };

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

		glTexGenfv(GL_S, GL_OBJECT_PLANE, planoS);
		glTexGenfv(GL_T, GL_OBJECT_PLANE, planoT);

		glColor4f(1, 1, 1, 1);
		glTranslatef(ojo[0], ojo[1], ojo[2]);
		glScalef(100, 100, 100);
		glRotatef(velocidad, 0, 0, 1);
		glRotatef(10, 0, 1, 0);
		glOrtho(1, -1, 1, -1, 1, - 1); // Camara ortografica
		glutSolidSphere(0.5, 20, 20);

		glPopMatrix();
		glPopAttrib();
	}
}
void crearVelocimetro() {
	if (verCabina) {

		float giroAguja = (abs(modulo) - velocidadMin) * 250 / (velocidadMax - velocidadMin) - 135;
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, velocimetro);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		//VELOCIMETRO
		glColor3f(1, 1, 1);
		glTranslatef(ojo[0], ojo[1], ojo[2]);

		glRotatef(velocidad + 20, 0, 0, 1);
		glRotatef(30, 0, 1, 0);
		glRotatef(-3, 0, 0, 1);

		glTranslatef(2, 0, 0);

		glScalef(-1, -1, 1);
		glScalef(2, 2, 2);

		glBegin(GL_POLYGON);

		glTexCoord2f(0, 0);
		glVertex3f(0, -0.1, -0.1);
		glTexCoord2f(0, 1);
		glVertex3f(0, -0.1, 0.1);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0.1, 0.1);
		glTexCoord2f(1, 0);
		glVertex3f(0, 0.1, -0.1);

		glEnd();

		glPopMatrix();
		glPopAttrib();

		//AGUJA
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, agujaVelocimetro);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor3f(1, 1, 1);
		glTranslatef(ojo[0], ojo[1], ojo[2]);
		glRotatef(velocidad + 20, 0, 0, 1);

		glRotatef(30, 0, 1, 0);
		glRotatef(-12, 0, 0, 1);

		glTranslatef(1.9, 0.3, 0);
		glScalef(2, 2, 2);

		glRotatef(giroAguja, 1, 0, 0);

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0, -0.1, -0.1);
		glTexCoord2f(0, 1);
		glVertex3f(0, -0.1, 0.1);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0.1, 0.1);
		glTexCoord2f(1, 0);
		glVertex3f(0, 0.1, -0.1);
		glEnd();

		glPopMatrix();
		glPopAttrib();
	}
}
void crearAlturometro() {
	if (verCabina) {

		float giroAguja = (ojo[2] - MIN_ALTURA) * 250 / (MAX_ALTURA -100) - 135;
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, alturometro);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		//VELOCIMETRO
		glColor3f(1, 1, 1);
		glTranslatef(ojo[0], ojo[1], ojo[2]);

		glRotatef(velocidad + 20, 0, 0, 1);
		glRotatef(30, 0, 1, 0);
		glRotatef(-18, 0, 0, 1);

		glTranslatef(2, 0, 0);

		glScalef(-1, -1, 1);
		glScalef(1.8, 1.8, 1.8);

		glBegin(GL_POLYGON);

		glTexCoord2f(0, 0);
		glVertex3f(0, -0.1, -0.1);
		glTexCoord2f(0, 1);
		glVertex3f(0, -0.1, 0.1);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0.1, 0.1);
		glTexCoord2f(1, 0);
		glVertex3f(0, 0.1, -0.1);

		glEnd();

		glPopMatrix();
		glPopAttrib();

		//AGUJA
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, agujaVelocimetro);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor3f(1, 1, 1);
		glTranslatef(ojo[0], ojo[1], ojo[2]);
		glRotatef(velocidad + 20, 0, 0, 1);

		glRotatef(30, 0, 1, 0);
		glRotatef(-27, 0, 0, 1);

		glTranslatef(1.9, 0.3, 0);
		glScalef(1.8, 1.8, 1.8);

		glRotatef(giroAguja, 1, 0, 0);

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0, -0.1, -0.1);
		glTexCoord2f(0, 1);
		glVertex3f(0, -0.1, 0.1);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0.1, 0.1);
		glTexCoord2f(1, 0);
		glVertex3f(0, 0.1, -0.1);
		glEnd();

		glPopMatrix();
		glPopAttrib();
	}
}
void crearBrujula() {
	if (verCabina) {
		float giroBrujula = velocidad;
		//BRUJULA
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, brujula);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor3f(1, 1, 1);
		glTranslatef(ojo[0], ojo[1], ojo[2]);
		glRotatef(velocidad - 20, 0, 0, 1);

		glRotatef(30, 0, 1, 0);
		glRotatef(3, 0, 0, 1);

		glTranslatef(1.9, 0, 0);
		glScalef(2, 2, 2);

		glRotatef(giroBrujula,1, 0, 0);

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0, -0.1, -0.1);
		glTexCoord2f(0, 1);
		glVertex3f(0, -0.1, 0.1);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0.1, 0.1);
		glTexCoord2f(1, 0);
		glVertex3f(0, 0.1, -0.1);
		glEnd();

		glPopMatrix();
		glPopAttrib();
	}
}
void crearAltitud() {
	if (verCabina) {
		static float dimension = EXTENSION / RESOLUCION_TERRENO;
		static float media = EXTENSION / 2;
		int posx = (ojo[0] + media) / dimension;
		int posy = (ojo[1] + media) / dimension;
		float alturamedia = max(altura[posx + 1][posy], altura[posx][posy + 1], altura[posx - 1][posy], altura[posx][posy - 1], altura[posx][posy]);
		float aux = (ojo[2] - MIN_ALTURA) - (alturamedia - MIN_ALTURA) - 20; 
		float giroAguja = aux * 250 / (MAX_ALTURA - 100) - 135;
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, alturometro);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		//VELOCIMETRO
		glColor3f(1, 1, 1);
		glTranslatef(ojo[0], ojo[1], ojo[2]);

		glRotatef(velocidad + 20, 0, 0, 1);
		glRotatef(43, 0, 1, 0);
		glRotatef(-15, 0, 0, 1);

		glTranslatef(2, 0, 0);

		glScalef(-1, -1, 1);
		glScalef(1.8, 1.8, 1.8);

		glBegin(GL_POLYGON);

		glTexCoord2f(0, 0);
		glVertex3f(0, -0.1, -0.1);
		glTexCoord2f(0, 1);
		glVertex3f(0, -0.1, 0.1);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0.1, 0.1);
		glTexCoord2f(1, 0);
		glVertex3f(0, 0.1, -0.1);

		glEnd();

		glPopMatrix();
		glPopAttrib();

		//AGUJA
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, agujaVelocimetro);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor3f(1, 1, 1);
		glTranslatef(ojo[0], ojo[1], ojo[2]);
		glRotatef(velocidad + 20, 0, 0, 1);

		glRotatef(43, 0, 1, 0);
		glRotatef(-24, 0, 0, 1);

		glTranslatef(1.9, 0.3, 0);
		glScalef(1.8, 1.8, 1.8);

		glRotatef(giroAguja, 1, 0, 0);

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0, -0.1, -0.1);
		glTexCoord2f(0, 1);
		glVertex3f(0, -0.1, 0.1);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0.1, 0.1);
		glTexCoord2f(1, 0);
		glVertex3f(0, 0.1, -0.1);
		glEnd();

		glPopMatrix();
		glPopAttrib();
	}
}

void display()
// Funcion de atencion al dibujo
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (noche) { glClearColor(0, 0.02, 0.06, 1); }
	if (!noche) { glClearColor(0.7, 0.65, 0.75,1); }
	// Seleccionar la MODELVIEW
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Situar y orientar la camara
	gluLookAt(ojo[0], ojo[1] , ojo[2], 100000 * cos((giroxTA + velocidad)*PI / 180) * abs(ojo[2]), 100000 * sin((giroxTA + velocidad) * PI / 180) * abs(ojo[2]), 100000 * dpZ * abs(ojo[2]), 0, 0, 1);

	// Iluminación Ambiente - Luna (por la noche)
	GLfloat F0[] = { 0, 0, 1, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, F0);

	//Foca direccional
	GLfloat apunta[] = { ojo[0],ojo[1],ojo[2],1 };
	glLightfv(GL_LIGHT1, GL_POSITION, apunta); // Luz focal sobre la camara
	GLfloat dc[] = { cos((giroxTA + velocidad) * PI / 180), sin((giroxTA + velocidad) * PI / 180), dpZ }; // Direccion del foco la de la vista
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, dc);

	//Focal Fija
	const float dpX2 = cos((velocidad) * PI / 180);
	const float dpY2 = sin((velocidad) * PI / 180);
	const float dpZ2 = -1;
	GLfloat apunta2[] = { ojo[0] + 400,ojo[1],ojo[2],1 };
	glLightfv(GL_LIGHT2, GL_POSITION, apunta2); // Luz focal sobre la camara
	GLfloat dc2[] = { dpX2, dpY2, dpZ2 }; // Direccion del foco la de la vista
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, dc2);

	generarTerreno();
	generarAgua();
	helipuerto();
	crearCabina();
	glPushMatrix();
	crearVelocimetro();
	crearBrujula();
	crearAlturometro();
	crearAltitud();

	static float dimension = EXTENSION / RESOLUCION_TERRENO;
	static float media = EXTENSION / 2;
	int posx = (ojo[0] + media) / dimension;
	int posy = (ojo[1] + media) / dimension;
	float posz = AutAltura + max((altura[posx + 1][posy] + altura[posx - 1][posy] + altura[posx][posy + 1] + altura[posx][posy - 1] / 4), altura[posx][posy]);
	
	stringstream vel;
	stringstream altitudTerreno;
	stringstream altitudOjo;
	stringstream pos;
	stringstream direccionBrujula;

	float alturamedia = max(altura[posx + 1][posy], altura[posx][posy + 1] , altura[posx - 1][posy], altura[posx][posy - 1], altura[posx][posy]);
	vel << "Velocidad " << modulo << " km/h";
	/*altitudOjo << "Altura " << ojo[2] + abs(MIN_ALTURA) -max(alturamedia,0) << " m";
	altitudTerreno << "Altitud " << ojo[2] + abs(MIN_ALTURA) << " m";*/
	altitudOjo << "Altura " << ojo[2] - MIN_ALTURA << " m";
	altitudTerreno << "Altitud " << (ojo[2] - MIN_ALTURA) - (alturamedia - MIN_ALTURA) -20  << " m";
	pos << "Posicion " << ojo[0] << ", " << ojo[1];
	direccionBrujula << "GPS " << ((int)velocidad % 360) << " grados";

	texto(0, 100,(char*)vel.str().c_str(), ROJO, (void*)8U, false);
	texto(0, 80, (char*)altitudOjo.str().c_str(), ROJO, (void*)8U, false);
	texto(0, 60, (char*)altitudTerreno.str().c_str(), ROJO, (void*)8U, false);
	texto(0, 40, (char*)pos.str().c_str(), ROJO, (void*)8U, false);
	texto(0, 20, (char*)direccionBrujula.str().c_str(), ROJO, (void*)8U, false);

	glPopMatrix();
	glutSwapBuffers();
}

void reshape(GLint w, GLint h)
// Funcion de atencion al redimensionamiento
{
	float relacionAspecto = float(w) / h;
	glViewport(0, 0, w, h);

	// Definir la camara
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60, relacionAspecto, 0.1, 100000);
}

void subir() {
	if (ojo[2] + 2 == 0) { ojo[2] += 3; }
	else if (ojo[2] -MIN_ALTURA <= MAX_ALTURA ) {
			ojo[2] += 2;
	}
}
void bajar() {
	if (ojo[2] - 2 == 0) { ojo[2] -= 3; }
	else if (ojo[2] >= MIN_ALTURA) {
			ojo[2] -= 2;
	}
}
void girarIzquierda() {
	velocidad += 1;
}
void girarDerecha() {
	velocidad -= 1; //Incremento de alpha
}
void acelerar() { 
	if (modulo < 280) {
		modulo += 1;
	}
}
void frenar() {
	if (modulo > 0 ) {
		modulo -= 1;
	}
}
void marchaAtras() {
	if (modulo > -60 && modulo <= 0) {
		modulo -= 1;
	}
}
void nocturno() {
	if (noche) {
		glDisable(GL_LIGHTING); //Deshabilitar luces
		GLfloat tono[] = { 0.7, 0.65, 0.75 };
		glFogfv(GL_FOG_COLOR, tono);
	}
	else {
		glEnable(GL_LIGHTING); //Habilitar luces
		GLfloat tono[] = { 0, 0, 0.05 };
		glFogfv(GL_FOG_COLOR, tono);
	}
	noche = !noche;
}
void luzFoco() {
	if (foco) {
		glDisable(GL_LIGHT1);
	}
	else {
		glEnable(GL_LIGHT1);
	}
	foco = !foco;
}
void cabinaVisible() {
	verCabina = !verCabina;
}
void pilotAuto() {
	autopilot = !autopilot;
	AutAltura = max(abs(ojo[2]), 150);
}
void modoMusica() {
	if (musica) {
		mciSendString(pause, NULL, 0, NULL);
		musica = false;
	}
	else {
		mciSendString(resume, NULL, 0, NULL);
		musica = true;
	}
}

void onKey(unsigned char tecla, int x, int y)
{
	// Callback de atencion a los eventos de teclas alfanumericas
	switch (tecla) {
	case 'A':
	case 'a':
		acelerar();
		break;
	case 'Z':
	case 'z':
		frenar();
		break;
	case 'R':
	case 'r':
		marchaAtras();
		break;
	case 'L':
	case 'l':
		nocturno();
		break;
	case 'F':
	case 'f':
		luzFoco();
		break;
	case 'Q':
	case 'q':
		pilotAuto();
		break;
	case 'C':
	case 'c':
		cabinaVisible();
		break;
	case 'M':
	case 'm':
		modoMusica();
		break;
	case 27:
		exit(0);
	}

	glutPostRedisplay();
}
void onSpecialKey(int tecla, int x, int y)
{
	// Callback de atencion a los eventos de teclas alfanumericas
	switch (tecla) {
	case GLUT_KEY_UP: 
		subir();
		break;
	case GLUT_KEY_DOWN:
		bajar();
		break;
	case GLUT_KEY_LEFT:
		girarIzquierda();
		break;
	case GLUT_KEY_RIGHT:
		girarDerecha();
		break;
	case 27:
		exit(0);
	}

	// Actualizacion de los angulos

	glutPostRedisplay();
}

void onClick(int boton, int estado, int x, int y)
{
	// Callback de atencion al pulsado de un boton del raton

	// Almacenar donde se hizo el click
	if (boton == GLUT_LEFT_BUTTON && estado == GLUT_DOWN) {
		xanterior = x;
		yanterior = y;
	}

	// Para preguntar por el pixel tengo que cambiar la Y de sentido
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	GLfloat vy = viewport[3] - y;
	glReadPixels(x, vy, 1, 1, GL_RED, GL_UNSIGNED_BYTE, objeto);
}

void onDrag(int x, int y)
{
	static const float pixel2grados = 1;
	static const int Xmax = 90;
	static const int Ymax = 45;
	static const int Ymin = -89;
	if (pulsado) { //
		pulsado = false;
	}
	else {
		// Al mover el raton hacia la derecha, la x aumenta y el giro es 
		// alrededor del eje y positivo
		giroxTA -= (x - xanterior) * pixel2grados;
		// AL mover el raton hacia abajo, la y aumenta y el giro es 
		// alrededor del eje x positivo
		giroyTA -= (y - yanterior) * pixel2grados;
	}

	if (giroxTA > Xmax) {
		giroxTA = Xmax;
	}
	else if (giroxTA < -Xmax) {
		giroxTA = -Xmax;
	}
	if (giroyTA >= Ymax) {
		giroyTA = Ymax;
	}
	else if (giroyTA < Ymin) {
		giroyTA = Ymin;
	}
	xanterior = x;
	yanterior = y;

	glutPostRedisplay();
}

void onIdle() {
	int posx = (ojo[0] + EXTENSION / 2) / (EXTENSION / RESOLUCION_TERRENO);
	int posy = (ojo[1] + EXTENSION / 2) / (EXTENSION / RESOLUCION_TERRENO);
	float posz = AutAltura + max((altura[posx + 1][posy] + altura[posx - 1][posy] + altura[posx][posy + 1] + altura[posx][posy - 1] / 4), altura[posx][posy]);
	//Calculamos el tiempo transcurrido desde la última vez
	static int antes1 = glutGet(GLUT_ELAPSED_TIME);
	int ahora1 = glutGet(GLUT_ELAPSED_TIME); //Tiempo transcurrido desde el inicio
	int tiempo_transcurrido = ahora1 - antes1; //Tiempo transcurrido desde antes en msg
	dpX = cos(( velocidad) * PI / 180); //giroxTA +
	dpY = sin((velocidad) * PI / 180); //giroxTA + 
	dpZ = tan((giroyTA)*PI / 180);
	ojo[0] += dpX * modulo * tiempo_transcurrido / 1000;
	ojo[1] += dpY * modulo * tiempo_transcurrido / 1000;

	//Piloto Automático - Mejora temblor
	if (autopilot) {

		if (ojo[2] >= posz && ojo[2] >= MIN_ALTURA + AutAltura) {
			ojo[2] -= 1;
		}
		else if (ojo[2] < posz) {
			ojo[2] += 1;
		}
		else if (ojo[2] > posz + 10 && ojo[2] >= MIN_ALTURA + AutAltura) {
			ojo[2] -= 5; 
		}
		else if (ojo[2] < posz - 10) {
			ojo[2] += 5;
		}
		
	}
	
	if (ojo[2] -50 < max(max(altura[posx + 1][posy], 
		altura[posx][posy + 1], 
		altura[posx - 1][posy],
		altura[posx][posy - 1],
		altura[posx][posy]), MIN_ALTURA)) { // Choque contra el suelo
		ojo[0] = 0;
		ojo[1] = 0;
		ojo[2] = ALTURA_INICIAL;
		modulo = 0;

		mciSendString(explosion, NULL, 0, NULL);
	}
	antes1 = ahora1;
	glutPostRedisplay();

}

void onTimer(int tiempo) {
	//Callback de atencion a la cuenta a tras de un timer encolarse a si misma
	glutTimerFunc(tiempo, onTimer, tiempo);
	onIdle();
}

void onMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		pulsado = true;
	}
}

int main(int argc, char** argv)
// Programa principal
{
	// Inicializaciones
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(PROYECTO);
	init();

	// Registro de callbacks	
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKey); //Solo para caracteres
	glutSpecialFunc(onSpecialKey); //Para las flechas
	glutTimerFunc(1000 / tasaFps, onTimer, 1000 / tasaFps);
	glutMouseFunc(onClick);
	glutMotionFunc(onDrag);

	// Bucle de atencion a eventos
	glutMainLoop();
}