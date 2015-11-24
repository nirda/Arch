#include <iostream>
#include <stdio.h>
#include <vector>
#include "ex1.h"
#include "math.h"
#include <unistd.h>
#include <string.h>

using namespace std;

////////////////////////////
// Functions definitions    
////////////////////////////

/********************************************************************
* Function  :  main
* Arguments : argc  : number of arguments in argv
* Returns   : argv  : command line arguments
* Throws    : n/a
*
* Purpose   : This is the main program function, It takes care of all the program flow.
*             It handles the command line arguments, creating the ArcBall, and throwing
*             everything else on the GLUT to handle.
*
\******************************************************************/

//define a gl window represented by struct
typedef struct {
	float width;
	float height;

	float curr_x;
	float curr_y;

	float field_of_view_angle;
	float z_near;
	float z_far;
} glutWindow;

//a 4X4 matrix that keep all the transformations that was done to move the object
GLfloat actions[16];

// a 4X4 matrix that is used every time for trasformation
GLfloat currRotateMat[16];

//last x and y mouse moves
float lastMovex;
float lastMovey;

//global variables used for the animation scene
vector<OpenMesh::Vec4f> origCoordinates;
bool animationWasDone = false;
float cubeMaxY;
Mesh::Point cubeCenter;

Mesh mesh;
float objectScale;
Mesh::Point cenDiff;

//is VBO mode on or off
bool drawVBO;

//is this the cube file or not
bool isCube;

//the VBO address
unsigned int vbo;

// variable that is used for the timer function for the animation
float tmp = 0;

glutWindow win;

//booleans that keep the mouse mode at every moment
bool isScaling  =false;
bool isRotating =false;
bool isMoving   =false;

int main(int argc, char * argv[]) 
{
  // check correct usage.
  if (argc != ARGUMENTS_REQUIRED)
  {
    fprintf(stderr, "Usage: %s <input_file>\n", argv[ARGUMENTS_PROGRAM]);
    return RC_INVALID_ARGUMENTS;
  }

  // set window values
  win.width = WINDOW_SIZE;
  win.height = WINDOW_SIZE;
  win.field_of_view_angle = 30;
  win.z_near = OBJ_DEPTH-OBJ_B_RAD;
  win.z_far = OBJ_DEPTH+OBJ_B_RAD;

  // Initialize GLUT.
  glutInit(&argc, argv) ;
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB) ;
  glutInitWindowSize(win.width, win.height);
  glutInitWindowPosition(WINDOW_POS_X, WINDOW_POS_Y);
  glutCreateWindow(argv[ARGUMENTS_PROGRAM]);

  // Initialize openGL  
  initGL();

  // Filling callback functions 
  glutDisplayFunc(display) ;
  glutReshapeFunc(reshape) ;
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);

  // try to load the mesh from the given input file 

  if (!OpenMesh::IO::read_mesh(mesh, argv[ARGUMENTS_INPUTFILE]))
  {
    // if we didn't make it, exit...  
    fprintf(stderr, "Error loading mesh, Aborting.\n");
    return RC_INPUT_ERROR;
  }


  printf("number of vertices is %d\n", mesh.n_vertices());
  printf("number of faces is %d\n", mesh.n_faces());
  printf("number of edges is %d\n", mesh.n_edges());

  Mesh::Point upper;
  Mesh::Point lower;
  computeCenterAndBoundingBox(&upper,&lower,&cenDiff);

  initObjectStartParams(&upper,&lower);

  //initialize VBO
  drawVBO = false;
  createVBOModel();

  isCube = false;
  string cubeName ("cube.off");
  if ( str_ends_with(argv[ARGUMENTS_INPUTFILE], cubeName.c_str()))
  {
	  isCube = true;
  }


  // enter the main loop  
  glutMainLoop();

  return RC_OK;
}

/********************************************************************
* Function  : timer
*
* Purpose   : this is a simple timer functions that is called recursively
* 				is animation starts.
*
\******************************************************************/
static void _Timer(int value)
{
	// if its the cube do standard animation
	if (isCube)
	{
		doCubeAnimation();
		tmp+=0.01;
	}
	// if it is not the cube we will same animation
	// of rotating
	else
	{
		doRotateAnimation();
		tmp+=0.3;
	}

  /* send redisplay event */
  glutPostRedisplay();

	if (tmp!=1)
	{
		  glutTimerFunc(50, _Timer, 0);
	}
  /* call this function again in 10 milliseconds */
}

/********************************************************************
* Function  : init
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This function is used to initialize some ultra important OpenGL settings.
*             Currently, it just takes care of the background color.
*
\******************************************************************/
void initGL(void)
{
	win.field_of_view_angle = START_ANGLE;
	  // Set the background color to black (a shocking surprise)
	  glClearColor (0.0, 0.0, 0.0, 0.0);
	  glShadeModel (GL_FLAT);

	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();
	  glGetFloatv(GL_MODELVIEW_MATRIX,actions);

	  gluLookAt(0,0,0,0,0,-1,0,1,0);

	  // select projection matrix
	   glMatrixMode(GL_PROJECTION);
	   glLoadIdentity();
	   GLfloat aspect = (GLfloat) win.width / win.height;
	   gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);
	   //gluPerspective(60,1,0.1,50);

		// set the viewport
		glMatrixMode(GL_VIEWPORT);
		glViewport(0, 0, win.width, win.height);
		// translate everything to be in OBJ_DEPTH //

		glMatrixMode(GL_MODELVIEW);
		glTranslatef(0.0,0.0, (-1)*OBJ_DEPTH);
	  return;
}


/********************************************************************
* Function  :  display
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This function is used to display the current animation frame.
*               
*
\******************************************************************/
void display(void) 
{

	glClear(GL_COLOR_BUFFER_BIT) ;

	drawModel();
	//drawCircle();

	// Make sure everything gets painted
	glFlush() ;
	// Swap those buffers so someone will actually see the results...
	glutSwapBuffers();
}


/********************************************************************
* Function  :  reshape
* Arguments : w  : new width of the window
*              h  : new height of the window
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : this function handles change of the window dimensions.
*
\******************************************************************/
void reshape(int w, int h) 
{
	return;
}


/********************************************************************
* Function  :  keyboard
* Arguments :  key : the key that was pressed
*             x   : x value of the current mouse location
*             y   : y value of the current mouse location
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This function handles all the keyboard input from the user.
*             It supports terminating the application when the KEY_QUIT is pressed.
*
\******************************************************************/
void keyboard(unsigned char key, int x, int y) 
{
	switch (key) {
		// move to vbo mode
		case 'v':
		{
			drawVBO = !drawVBO;
			break;
		}
		// start animation
		case 'a':
		{
			tmp = 0;
			if (isCube)
			{
				prepareForAnimation();
			}
			glutTimerFunc(10, _Timer, 0); /* register callback for a timer */
			break;
		}
		// Reset the object, i.e. start from the begining
		case 'r':
		{
			initGL();
			glutPostRedisplay();

			break;
		}
		// Quit game
		case 'q':
		{
			exit(0);
			break;
		}
		default:
			break;
	}

  return;
}


/********************************************************************
* Function  :   mouse 
* Arguments :   button  : the button that was engaged in some action
*               state   : the new state of that button
*               x       : x value of the current mouse location
*               y       : y value of the current mouse location
* Returns   :   n/a
* Throws    :   n/a
*
* Purpose   :   This function handles mouse actions.
*
\******************************************************************/
void mouse(int button, int state, int x, int y) {
    win.curr_x=x;
    win.curr_y=y;

    // Handle left button of the mouse, i.e. rotating
    if(button == GLUT_LEFT_BUTTON) {
    	// Keep the start coordinates and initialize
    	// rotating flag
        if(state == GLUT_DOWN) {
            lastMovex=x;
            lastMovey=y;
            isRotating = true;
        }
        // Finish rotating - keep the long current rotation
        // in the actions matrix
        else {
            updateActionsMatrix();
            isRotating = false;
        }
    }
    // Handle middle button of the mouse, i.e. scaling
    else if (button==GLUT_MIDDLE_BUTTON)
    {
        if(state == GLUT_DOWN) {
            isScaling = true;
        }
        else {
            isScaling = false;
        }
    }
    // Handle right button of the mouse, i.e. moving
    else if (button==GLUT_RIGHT_BUTTON)
    {
    	// Keep the start coordinates and initialize
    	// moving flag
        if(state == GLUT_DOWN) {
            lastMovex=x;
            lastMovey=y;
            isMoving = true;
        }
        // Finish moving - keep the long current movement
        // in the actions matrix
        else {
            updateActionsMatrix();
            isMoving = false;
        }
    }
}

/********************************************************************
* Function  :   motion  
* Arguments :   x   : x value of the current mouse location
*               y   : y value of the current mouse location
* Returns   :   n/a
* Throws    :   n/a
*
* Purpose   :   This function handles mouse dragging events.
*
* 				we use the "currRotateMat" matrix for displaying transformations.
* 				the actions matrix saves all action done only at the end (when mouse is up)
*
\******************************************************************/
void motion(int x, int y)
{
	// Do something only if we actually move
    if ((x!=win.curr_x)||(y!=win.curr_y))
    {
        float newX= OBJ_RADIUS*x/win.width*2 - OBJ_RADIUS;
        float newY = -((OBJ_RADIUS*y/win.height*2) - OBJ_RADIUS);
        float prevX= OBJ_RADIUS*lastMovex/win.width*2 - OBJ_RADIUS;
        float prevY = -((OBJ_RADIUS*lastMovey/win.height*2) - OBJ_RADIUS);
        float diffX = newX-prevX;
        float diffY = newY-prevY;

        // Scaling the object using the angle of perspective
        if (isScaling)
        {
        	// Scaling down
            if (y>win.curr_y&&win.field_of_view_angle>1)
            {
                   glMatrixMode(GL_PROJECTION);
                   glLoadIdentity();
                   --win.field_of_view_angle;
                   GLfloat aspect = (GLfloat) win.width / win.height;
                   gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);
                   glMatrixMode(GL_MODELVIEW);
                   glutPostRedisplay();
            }
            // Scaling up
            else if (y<win.curr_y&&win.field_of_view_angle<179)
            {
                glMatrixMode(GL_PROJECTION);
                 glLoadIdentity();
                 ++win.field_of_view_angle;
                GLfloat aspect = (GLfloat) win.width / win.height;
                gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);
                glMatrixMode(GL_MODELVIEW);
                glutPostRedisplay();
            }
        }
        // Handling rotating case
        // Displaying motion on screen BUT NOT inserting it
        // to the actions matrix
        else if (isRotating)
        {
            float z1=getZVal(prevX,prevY,OBJ_RADIUS);
            float z2=getZVal(newX,newY,OBJ_RADIUS);

            OpenMesh::Vec3f v2( newX, newY, z2);
            OpenMesh::Vec3f v1(prevX,prevY,z1);

            OpenMesh::Vec3f normal=v1%v2;

            float angle = calcAngle(v1,v2);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glRotatef(2*angle,normal[0],normal[1],normal[2]);
            glGetFloatv(GL_MODELVIEW_MATRIX,currRotateMat);
            glLoadIdentity();
            glTranslatef(0,0,(-1)*OBJ_DEPTH);
            glMultMatrixf(currRotateMat);
            glMultMatrixf(actions);
            glutPostRedisplay();
        }
        // Handling moving case
        // Displaying motion on screen BUT NOT inserting it
        // to the actions matrix
        else if (isMoving)
        {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(diffX*2,diffY*2,0);
            glGetFloatv(GL_MODELVIEW_MATRIX,currRotateMat);
            glLoadIdentity();
            glTranslatef(0,0,(-1)*OBJ_DEPTH);
            glMultMatrixf(currRotateMat);
            glMultMatrixf(actions);
            glutPostRedisplay();
        }

        win.curr_x=x;
        win.curr_y=y;
    }
  return;
}

/********************************************************************
* Function  : updateActionsMatrix()
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This method updates the action matrix after every action.
* 			   taht way we do not accumulate transformations one during
* 			   motion event, but display them. Accumulate only when the mouse button is released.
*
*
\******************************************************************/
void updateActionsMatrix() {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixf(currRotateMat);
    glMultMatrixf(actions);
    glGetFloatv(GL_MODELVIEW_MATRIX,actions);
    glPopMatrix();
}


/********************************************************************
* Function  : drawModel
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This method is incharge od drawing the mesh model to the screen
* 				it draw in VBO mode or regular mode according to user's choice.
*
*
\******************************************************************/
void drawModel()
{
	glColor3f(1.0,0.0,0.0);
	if (!drawVBO)
	{
		Mesh::EdgeIter eIter;
		Mesh::EdgeHandle eHandle;
		Mesh::VertexHandle vHandle1,vHandle2;
		Mesh::Point p1, p2;
		// we could have used Point for this as well, but in order to
		// show the difference between a point and a direction vector we'll use Normal.
		Mesh::Normal edgeVector;

		for (eIter = mesh.edges_begin();eIter != mesh.edges_end(); ++eIter)
		{
			eHandle = eIter.handle();

			vHandle1 = mesh.from_vertex_handle(mesh.halfedge_handle(eHandle,0));

			/* and the vertex to which the halfedge points */
			vHandle2 = mesh.to_vertex_handle(mesh.halfedge_handle(eHandle,0));

			/* now that we have the handles we can access the points */
			p1 = objectScale*(mesh.point(vHandle1)-cenDiff);
			p2 = objectScale*(mesh.point(vHandle2)-cenDiff);
			//std::cout << "curr : p1 :" << p1 << " p2: " << p2 << std::endl;

			glBegin(GL_LINES);
			glVertex3f(p1[0],p1[1],p1[2]);
			glVertex3f(p2[0],p2[1],p2[2]);
			glEnd();
		}
	}
	else
	{
		drawVBOModel();
	}

}

/********************************************************************
* Function  : drawCircle
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This method draws the arcball circle to the screen.
*
*
\******************************************************************/
void drawCircle()
{
	// Updating projection  in order to draw a regular circle on
	// screen
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, win.width, win.height, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_CULL_FACE);

	glClear(GL_DEPTH_BUFFER_BIT);

	// Draw circle
	float x1,y1;
	float angle;
	double radius=win.width/2-10;

	x1 = win.width/2,y1=win.height/2;
	glColor3f(0.0,1.0,0.0);

	 glBegin(GL_LINE_LOOP);
	 for (angle = 0; angle< 2*M_PI; angle+=0.1)
	 {
	 	glVertex2f(x1 + sin(angle) * radius, y1 + cos(angle) * radius);
	 }
	 glEnd();


	// Making sure we can render 3d again
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

/********************************************************************
* Function  : createVBOModel()
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : create the VBO model that will be saved fo later use.
*
*
\******************************************************************/
void createVBOModel()
{
	Mesh::VertexIter vertexIter;
	vertexIter = mesh.vertices_begin();
	int edgesNum = mesh.n_edges();
	int arrSize = 6*edgesNum;
	float* verArr = NULL;
	verArr = new float[arrSize];

	// create an array of floats containing 3 floats for every vertex in the mesh object.
	Mesh::EdgeIter eIter;
	Mesh::EdgeHandle eHandle;
	Mesh::VertexHandle vHandle1,vHandle2;
	Mesh::Point p1, p2;
	int i=0;
	for (eIter = mesh.edges_begin();eIter != mesh.edges_end(); ++eIter)
	    {
	      eHandle = eIter.handle();
	      vHandle1 = mesh.from_vertex_handle(mesh.halfedge_handle(eHandle,0));
	      vHandle2 = mesh.to_vertex_handle(mesh.halfedge_handle(eHandle,0));
	      p1 = objectScale*(mesh.point(vHandle1)-cenDiff);
	      p2 = objectScale*(mesh.point(vHandle2)-cenDiff);
	      for (int j=0; j<3; j++)
	      {
	    	  verArr[i + j] = p1[j];
	      }
	      i+=3;
	      for (int j=0; j<3; j++)
	      {
	    	  verArr[i + j] = p2[j];
	      }
	      i+=3;
	    }

	//create the VBO object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, arrSize * sizeof(float),verArr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

/********************************************************************
* Function  : drawVBOModel()
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This method draw the mesh object in VBO mode
*
*
\******************************************************************/
void drawVBOModel()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (char *)NULL);
	glDrawArrays(GL_LINES, 0, 6*mesh.n_edges());
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/********************************************************************
* Function  : prepareForAnimation()
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This method prepares for the cube animation.
* 				it finds the upper four vertex of the cube.
* 				it calculates the center point which 4 upper vertex  should go to.
* 				it saves the original vertex in order the play the animation again.
* 				if animation was already done once it returns the four upper vertex to their original position
* 				before we start the animation.
*
\******************************************************************/
void prepareForAnimation()
{
	Mesh::VertexIter vertexIter = mesh.vertices_begin();

	// If we have done animation before we need to perform it
	// from the original place of vertices
	if(!animationWasDone)
	{
		OpenMesh::Vec3f point;

		float currY;

		point= mesh.point(vertexIter.handle());
		float maxY =point[1];

		// Get max y coordinate, the wanted 4 vertices are those
		// with higher y coordinate
		for (vertexIter = mesh.vertices_begin();vertexIter != mesh.vertices_end();++vertexIter)
		{
			point= mesh.point(vertexIter.handle());
			currY = point[1];

			if (currY > maxY)
			{
				maxY = currY;
			}

		}
		cubeMaxY = maxY;

		Mesh::Point center(0,0,0);
		Mesh::Point p;
		int ind=0;

		// Find the center of the 4 upper vertices
		for (vertexIter = mesh.vertices_begin();vertexIter != mesh.vertices_end();++vertexIter)
		{
			p = mesh.point(vertexIter);
			if (p[1]==maxY)
			{
				center += p;
				OpenMesh::Vec4f keepOldVertex;
				keepOldVertex[0]=p[0];
				keepOldVertex[1]=p[1];
				keepOldVertex[2]=p[2];
				keepOldVertex[3]=ind;
				origCoordinates.push_back(keepOldVertex);

			}
			ind++;
		}
		center = center/4.0;
		cubeCenter = center;
		animationWasDone = true;
	}
	// Return to original place of vertices if animation was done already
	else
	{
		unsigned int j;
		j=0;
		for (vertexIter = mesh.vertices_begin();vertexIter != mesh.vertices_end();++vertexIter)
		{
			OpenMesh::Vec3f currPoint;
			currPoint = mesh.point(vertexIter.handle());
			if (currPoint[1]==cubeMaxY)
			{
				OpenMesh::Vec3f newCoordinate(origCoordinates[j][0],origCoordinates[j][1],origCoordinates[j][2]);
				j++;
				mesh.set_point(vertexIter.handle() , newCoordinate);

			}
		}
		glutPostRedisplay();
	}
}

/********************************************************************
* Function  : doCubeAnimation()
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This method does the "cube.off" animation. after finding top vertex of the cube
* 				and their center we move them slowly to the center point.
* 				this method is called in the "_Timer" method.
* 				while the "tmp" variable goes slowly from 0 to 1.
*
*
\******************************************************************/
void doCubeAnimation()
{
	// Calc new position of upper vertices using the formula
	// (1-t)*p + t*center
	// while t is globalic in the range of [0,1]

	Mesh::VertexIter vertexIter = mesh.vertices_begin();
	for (vertexIter = mesh.vertices_begin();vertexIter != mesh.vertices_end();++vertexIter)
	{
		OpenMesh::Vec3f newCoordinate;
		newCoordinate = mesh.point(vertexIter.handle());
		if (newCoordinate[1]==cubeMaxY)
		{
			newCoordinate = (1-tmp)*newCoordinate + cubeCenter*tmp;
			mesh.set_point(vertexIter.handle() , newCoordinate);

		}
	}

	glutPostRedisplay();
}

/********************************************************************
* Function  : doCubeAnimation()
* Arguments : n/a
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : This method rotate the object as part of the animation
\******************************************************************/
void doRotateAnimation()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(2*tmp,0,1,0);
    glGetFloatv(GL_MODELVIEW_MATRIX,currRotateMat);
    glLoadIdentity();
    glTranslatef(0,0,(-1)*OBJ_DEPTH);
    glMultMatrixf(currRotateMat);
    glMultMatrixf(actions);
    glutPostRedisplay();
}
////////////////////////////
// OpenMesh usage examples  
////////////////////////////

using namespace std;

/*
  This function computes the geometrical center and
  the axis aligned bounding box of the object.
  The bounding box is represented by the lower left and upper right
  corners.
*/
void computeCenterAndBoundingBox(Mesh::Point* upper,
								 Mesh::Point* lower,
								 Mesh::Point* cen)
{
  /* Vertex iterator is an iterator which goes over all the vertices of the mesh */
  Mesh::VertexIter vertexIter;

  /* This is the specific class used to store the geometrical position of the vertices of the mesh */
  Mesh::Point p;
  Mesh::Point lowerLeft(0,0,0);
  Mesh::Point upperRight(0,0,0);
  Mesh::Point center;

  /* number of vertices in the mesh */
  int vNum = mesh.n_vertices();


  vertexIter = mesh.vertices_begin();
  lowerLeft = upperRight = mesh.point(vertexIter);

  /* This is how to go over all the vertices in the mesh */
  for (vertexIter = mesh.vertices_begin();vertexIter != mesh.vertices_end();++vertexIter)
    {
      /* this is how to get the point associated with the vertex */
      p = mesh.point(vertexIter);
      center += p;
      for (int i = 0;i < 3;i++)
        {
          if (lowerLeft[i] > p[i])
            lowerLeft[i] = p[i];
          if (upperRight[i] < p[i])
            upperRight[i] = p[i];
        }
    }
  center /= (double)vNum;

  *upper=upperRight;
  *lower = lowerLeft;
  *cen=center;
}


/**
   This function count how many vertices a face has
*/
void faceValenceCounter(Mesh& mesh)
{
  /* Face iterator */
  Mesh::FaceIter   fIter;
  /*In order to get information about a certain face (or edge or vertex) you need to get the handle for this entity. */
  /* once you have this handle you get ask the mesh object to give you information or to do something (nice) for you.*/
  Mesh::FaceHandle faceH;
  for (fIter = mesh.faces_begin();fIter != mesh.faces_end(); ++fIter)
    {

      /* In this case we use the handle to get the index of the face and the number of vertices that this face has. */
      faceH = fIter.handle();
      cout <<"face #"<<faceH.idx()<<" has "<<mesh.valence(faceH)<<" vertices"<<endl;
    }
}

/**
   This function iterates over all the edges and computes their length.
   It does acheives this in two different ways - for educational purposes...
*/
void edgesLengths(Mesh& mesh)
{
  Mesh::EdgeIter eIter;
  Mesh::EdgeHandle eHandle;
  Mesh::VertexHandle vHandle1,vHandle2;
  Mesh::Point p1, p2;
  // we could have used Point for this as well, but in order to
  // show the difference between a point and a direction vector we'll use Normal.
  Mesh::Normal edgeVector;

  double edgeLen1, edgeLen2;

  for (eIter = mesh.edges_begin();eIter != mesh.edges_end(); ++eIter)
    {
      eHandle = eIter.handle();
      /* the easy way - ask for the edge's length */
      edgeLen1 = mesh.calc_edge_length(eHandle);

      /* The hard way - obtain the 2 vertices that the edge connects and measure the distance between them */
      /**
         Each edge is built from 2 directed 'half edges' one from vertex a to b and the other from b to a.
         To get the first of these halfedges we use the mesh.halfedge_handle(eHandle,0)
         To get the second of these halfedges we use the mesh.halfedge_handle(eHandle,1).
         Actually we don't need the both of them in this case...
         Once we have the half edge hanlde we can get the vertex from which the halfedge leaves
      */
      vHandle1 = mesh.from_vertex_handle(mesh.halfedge_handle(eHandle,0));

      /* and the vertex to which the halfedge points */
      vHandle2 = mesh.to_vertex_handle(mesh.halfedge_handle(eHandle,0));

      /* now that we have the handles we can access the points */
         p1 = mesh.point(vHandle1);
         p2 = mesh.point(vHandle2);
         edgeVector = p2 - p1;
         edgeLen2 = edgeVector.norm();
         cout <<"#: "<<eHandle.idx()<<" edge length (easy way) "<< edgeLen1 <<" edge length (hard way) "<< edgeLen2 << endl;

    }
}

/********************************************************************
* Function  : initObjectStartParams
* Arguments : Mesh::Point* upper, Mesh::Point* lower
* Returns   : n/a
* Throws    : n/a
*
* Purpose   : this method initializees the objectScale value parameter. in order to draw it in the right size
*
\******************************************************************/

void initObjectStartParams(Mesh::Point* upper,
		 	 	 	 	   Mesh::Point* lower)
{
	Mesh::Point edge = *upper-*lower;
	double len = edge.norm();
	objectScale=4*OBJ_RADIUS/len;
}

/* this function computes the center of each mesh */
void faceCenter(Mesh & mesh)
{
  /* face iterator */
  Mesh::FaceIter fIter;
  Mesh::FaceHandle fHandle;
  /* face vertex iterator: this iterator goes through all the vertices which belong to the face */
  /* This special type of iterator is called circulator in OpenMesh terms */
  /* There are many types of circulators such as VertexVertex or FaceEdge VertexFace etc. */
  Mesh::FaceVertexIter fvIter;


  /* go over all the faces */
  for (fIter = mesh.faces_begin();fIter != mesh.faces_end(); ++fIter)
    {
      fHandle = fIter.handle();
      Mesh::Point center(0,0,0);
      int faceVertexNum = mesh.valence(fHandle);

      /* for each face - go over all the vertices that belong to it and compute their average position (center) of face */
      /* notice the termination condition of this iterator */
      for (fvIter = mesh.fv_iter(fHandle);fvIter;++fvIter)
        {
          center += mesh.point(fvIter);
        }
      center /= faceVertexNum;
      cout <<"center of face "<<fHandle.idx()<< " is "<< center<<endl;
    }
}

void vectorDemo()
{
  cout <<"Linear algebra with OpenMesh's vector class demo "<<endl;
  /*a 3D vector of type double - there are also Vec3f, Vec2f, Vec2i,... */
  /* actually MyMesh::Point and MyMesh::Normalt are actaully Vec3f (in our case) */
  OpenMesh::Vec3d a(0.1,0.2,0.3), b(0.1,0.3,0.5);
  cout <<"a is "<<a<< endl<<"b is "<< b<<endl;
  // assignment

  cout <<"assigning a[2] = 0.75"<<endl;
  a[2] = 0.75;
  cout <<"now, a is "<< a<<endl;
  cout <<"a + b ="<<a + b<<endl;
  cout <<"dot product of a and b is "<< (a | b) <<endl;
  cout <<"cross product of a and b is "<< a % b << endl;
  cout <<"norm of a is "<< a.norm()<<endl;
  a.normalize();
  cout << "after normalziation a is "<< a<<" now, norm of a is "<< a.norm()<<endl;
  cout <<"==========================================================="<<endl;
}

/********************************************************************
* Function  : getZVal
* Arguments : float x, float y, float radius
* Returns   : float - the Z value according to the given x, y, and radius
* Throws    : n/a
*
* Purpose   : calculate the z value.
*
\******************************************************************/

/********************************************************************
* Function  : getZVal
* Arguments : float coordinates of x,y
* Returns   : float - z coordinate
* Throws    : n/a
*
* Purpose   : calculate the worlds z coordinates of given x,y

*
\******************************************************************/
float getZVal(float x, float y, float radius)
{
	float z;

	float x2=x*x;
	float y2=y*y;
	float r2=radius*radius;

	if (x2+y2>=r2)
	{
		z=0;
	}
	else
	{
		z=sqrt(r2-x2-y2);
	}

	return z;
}

/********************************************************************
* Function  : calaAngle
* Arguments : OpenMesh::Vec3f v1, OpenMesh::Vec3f v2
* Returns   : float - the angle between two vectors v1, v2
* Throws    : n/a
*
* Purpose   : calculate the angle between two given vectors

*
\******************************************************************/

float calcAngle(OpenMesh::Vec3f v1, OpenMesh::Vec3f v2)
{
	float result=v1|v2;
	result/=v1.norm();
	result/=v2.norm();
	result=acos(std::min(result,1.0f))*180.0/M_PI;
	return result;
}

/*  returns 1 iff str ends with suffix  */
int str_ends_with(const char * str, const char * suffix) {

  if( str == NULL || suffix == NULL )
    return 0;

  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  if(suffix_len > str_len)
    return 0;

  return 0 == strncmp( str + str_len - suffix_len, suffix, suffix_len );
}


