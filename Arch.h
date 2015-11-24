

#ifndef __EX1_H__
#define __EX1_H__


////////////////////////////
// Project Includes         
////////////////////////////

#include <stdlib.h>
#include <ctype.h>
#include <math.h>


////////////////////////////
// OpenMesh Includes        
////////////////////////////

#include "OpenMesh/Core/IO/MeshIO.hh"
#include "OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh"

////////////////////////////
// GL Includes              
////////////////////////////

#include "GLee.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

////////////////////////////
// Defines                  
////////////////////////////

#define RC_OK                 (0) // Everything went ok                        
#define RC_INVALID_ARGUMENTS  (1) // Invalid arguments given to the program    
#define RC_INPUT_ERROR        (2) // Invalid input to the program              

#define  ARGUMENTS_PROGRAM     (0) // program name position on argv            
#define  ARGUMENTS_INPUTFILE   (1) // given input file position on argv        
#define  ARGUMENTS_REQUIRED    (2) // number of required arguments             

#define  WINDOW_SIZE         (700) // initial size of the window               
#define  WINDOW_POS_X        (100) // initial X position of the window         
#define  WINDOW_POS_Y        (100) // initial Y position of the window
#define  OBJ_RADIUS          (10)
#define OBJ_DEPTH			 (75)
#define OBJ_B_RAD			 (70)
#define START_ANGLE			 (30)

////////////////////////////
// Key Definitions      
////////////////////////////

#define KEY_QUIT            ('q') // Key used to terminate the program         
#define KEY_RESET           ('r') // Key used to reset the applied TX's        


////////////////////////////
// Type Definitions         
////////////////////////////

// Mesh object  
typedef OpenMesh::PolyMesh_ArrayKernelT<> Mesh;

// Vector of 3 floats  
typedef OpenMesh::VectorT<float, 3> Vector3F;


////////////////////////////
// Functions declarations   
////////////////////////////

// initialize openGL settings 
void initGL(void);

// display callback 
void display(void);

// window reshape callback  
void reshape(int width, int height);

// keyboard callback  
void keyboard(unsigned char key, int x, int y);

// mouse click callback 
void mouse(int button, int state, int x, int y) ;

OpenMesh::Vec3d worldVector(float x, float y, float z);

// mouse dragging callback  
void motion(int x, int y) ;

//drawing the model to screen
void drawModel();

//drawing the arcbll 2D circle
void drawCircle();

//return Z value of a circle
float getZVal(float x, float y, float radius);

//calculate the angle between two vectors
float calcAngle(OpenMesh::Vec3f v1, OpenMesh::Vec3f v2);

//draw the model with VBO
void drawVBOModel();

//create a VBO to draw the model
void createVBOModel();

//prepare for the animation scene
void prepareForAnimation();

//do the animation scene
void doCubeAnimation();

void doRotateAnimation();

int str_ends_with(const char * str, const char * suffix);

//a timer func for running the animation scene
static void _Timer(int value);

//update the matrix that keeps all actions
void updateActionsMatrix();

///////////////////////////
// OpenMesh usage examples 
///////////////////////////
void faceCenter(Mesh & mesh);

// Compute the center and bounding box of objects
void computeCenterAndBoundingBox(Mesh::Point* upper,
		 	 	 	 	 	 	 Mesh::Point* lower,
		 	 	 	 	 	 	 Mesh::Point* cen);

// Init the center diff and scaling initialize parameters
void initObjectStartParams(Mesh::Point* upper,
		 	 	 	 	   Mesh::Point* lower);

void faceValenceCounter(Mesh& mesh);
void edgesLengths(Mesh& mesh);
void faceCenter(Mesh & mesh);
void vectorDemo();

#endif
