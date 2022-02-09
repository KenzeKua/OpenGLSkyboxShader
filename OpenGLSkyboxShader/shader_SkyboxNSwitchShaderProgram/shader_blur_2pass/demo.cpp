#define GLFW_INCLUDE_ES2 1
#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include "angle_util/Matrix.h"
#include "angle_util/geometry_utils.h"
#include "bitmap.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define TEXTURE_COUNT 7

static float highPassLimit = 0.5;
static float rotate = 0.0;

GLint GprogramID1 = -1;
GLint GTwistProgramID = -1;
GLint GSkyBoxProgramID = -1;

GLuint GtextureID[TEXTURE_COUNT];
GLuint GCubeMaptextureID;

GLuint Gframebuffer;
GLuint GdepthRenderbuffer;

GLuint GfullscreenTextureMain;
// = = =
GLuint GfullscreenTextureHighpass;
GLuint GfullscreenTextureBlur1;
GLuint GfullscreenTextureBlur2;
// = = =
GLuint GfullscreenTextureGray;

GLuint GtextureBG;
GLuint GdepthTextureBG;
GLint GtexBGWidth = WINDOW_WIDTH / 1;
GLint GtexBGHeight = WINDOW_HEIGHT / 1;

GLFWwindow* window;

Matrix4 gPerspectiveMatrix;
Matrix4 gViewMatrix;

#define RECT_VERTICE_W 6
#define RECT_VERTICE_H 6

// Every quad has 6 vertices
const int RECT_VERTEX_COUNT = RECT_VERTICE_W * RECT_VERTICE_H * 6;
// Every vertex has 3 components (x, y z)
const int RECT_VERTEX_ARRAY_SIZE = RECT_VERTEX_COUNT * 3;

const int RECT_UV_ARRAY_SIZE = RECT_VERTEX_COUNT * 2;

GLfloat mRectVertices[RECT_VERTEX_ARRAY_SIZE];
GLfloat mRectUV[RECT_UV_ARRAY_SIZE];

float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float cameraDistance = 18.0f;

bool isGrayOut = true;
bool isBloomOut = true;

void genPlane(void)
{
	const float width = 2.0f;
	const float height = 2.0f;
	const float halfWidth = width * 0.5f;
	const float halfHeight = height * 0.5f;

	const float texMul = 1.0f;

	int currentVert = -1;
	int currentIndex = -1;
	for (int h = 0; h < RECT_VERTICE_H; h++)
	{
		for (int w = 0; w < RECT_VERTICE_W; w++)
		{
			// ======== 6 vertices to form one sub-rectangle
			// 1st vertex
			int vertex1 = ++currentVert;
			mRectVertices[vertex1 * 3] = -halfWidth + (float)(w) / (float)RECT_VERTICE_W * width;
			mRectVertices[vertex1 * 3 + 1] = -halfHeight + (float)(h) / (float)RECT_VERTICE_H * height;
			mRectVertices[vertex1 * 3 + 2] = 0.0f;
			mRectUV[vertex1 * 2] = (float)(w) / (float)RECT_VERTICE_W * texMul;
			mRectUV[vertex1 * 2 + 1] = (float)(h) / (float)RECT_VERTICE_H* texMul;

			// 2nd vertex
			int vertex2 = ++currentVert;
			mRectVertices[vertex2 * 3] = -halfWidth + (float)(w) / (float)RECT_VERTICE_W * width;
			mRectVertices[vertex2 * 3 + 1] = -halfHeight + (float)(h + 1) / (float)RECT_VERTICE_H * height;
			mRectVertices[vertex2 * 3 + 2] = 0.0f;
			mRectUV[vertex2 * 2] = (float)(w) / (float)RECT_VERTICE_W* texMul;
			mRectUV[vertex2 * 2 + 1] = (float)(h + 1) / (float)RECT_VERTICE_H* texMul;


			// 3rd vertex
			int vertex3 = ++currentVert;
			mRectVertices[vertex3 * 3] = -halfWidth + (float)(w + 1) / (float)RECT_VERTICE_W * width;
			mRectVertices[vertex3 * 3 + 1] = -halfHeight + (float)(h + 1) / (float)RECT_VERTICE_H * height;
			mRectVertices[vertex3 * 3 + 2] = 0.0f;
			mRectUV[vertex3 * 2] = (float)(w + 1) / (float)RECT_VERTICE_W * texMul;
			mRectUV[vertex3 * 2 + 1] = (float)(h + 1) / (float)RECT_VERTICE_H * texMul;


			// 4th vertex
			int vertex4 = ++currentVert;
			mRectVertices[vertex4 * 3] = mRectVertices[vertex3 * 3];
			mRectVertices[vertex4 * 3 + 1] = mRectVertices[vertex3 * 3 + 1];
			mRectVertices[vertex4 * 3 + 2] = mRectVertices[vertex3 * 3 + 2];
			mRectUV[vertex4 * 2] = mRectUV[vertex3 * 2];
			mRectUV[vertex4 * 2 + 1] = mRectUV[vertex3 * 2 + 1];


			// 5th vertex
			int vertex5 = ++currentVert;
			mRectVertices[vertex5 * 3] = -halfWidth + (float)(w + 1) / (float)RECT_VERTICE_W * width;
			mRectVertices[vertex5 * 3 + 1] = -halfHeight + (float)(h) / (float)RECT_VERTICE_H * height;
			mRectVertices[vertex5 * 3 + 2] = 0.0f;
			mRectUV[vertex5 * 2] = (float)(w + 1) / (float)RECT_VERTICE_W * texMul;
			mRectUV[vertex5 * 2 + 1] = (float)(h) / (float)RECT_VERTICE_H * texMul;

			// 6th vertex
			int vertex6 = ++currentVert;
			mRectVertices[vertex6 * 3] = mRectVertices[vertex1 * 3];
			mRectVertices[vertex6 * 3 + 1] = mRectVertices[vertex1 * 3 + 1];
			mRectVertices[vertex6 * 3 + 2] = mRectVertices[vertex1 * 3 + 2];
			mRectUV[vertex6 * 2] = mRectUV[vertex1 * 2];
			mRectUV[vertex6 * 2 + 1] = mRectUV[vertex1 * 2 + 1];
		}
	}
}

void drawSkyBox()
{
	static float skyboxVertices[] =
	{      
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, skyboxVertices);

	glEnableVertexAttribArray(0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, GCubeMaptextureID);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisableVertexAttribArray(0);
}

static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

GLuint LoadShader(GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
		 char infoLog[4096];
         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         printf ( "Error compiling shader:\n%s\n", infoLog );            
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;
}

GLuint LoadShaderFromFile(GLenum shaderType, std::string path)
{
    GLuint shaderID = 0;
    std::string shaderString;
    std::ifstream sourceFile( path.c_str() );

    if( sourceFile )
    {
        shaderString.assign( ( std::istreambuf_iterator< char >( sourceFile ) ), std::istreambuf_iterator< char >() );
        const GLchar* shaderSource = shaderString.c_str();

		return LoadShader(shaderType, shaderSource);
    }
    else
        printf( "Unable to open file %s\n", path.c_str() );

    return shaderID;
}

void loadTexture(const char* path, GLuint textureID)
{
	CBitmap bitmap(path);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

	// bilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.GetWidth(), bitmap.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetBits());
}

void loadCubemapTexture(std::vector<std::string> facesPath, GLuint textureID)
{
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for(int i=0; i<facesPath.size(); i++)
	{
		CBitmap bitmap(facesPath[i].c_str());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, bitmap.GetWidth(), bitmap.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetBits());
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glDisable(GL_TEXTURE_CUBE_MAP);
}

int Init ( void )
{
   GLuint vertexShader1;
   GLuint fragmentShader1;
   GLuint programObject1;

   GLuint vertexShader2;
   GLuint fragmentShader2;
   GLuint programObject2;

   GLuint vertexShader3;
   GLuint fragmentShader3;
   GLuint programObject3;

   GLint linked;

   // Load textures
   glGenTextures(TEXTURE_COUNT, GtextureID);
   loadTexture("../media/rocks.bmp", GtextureID[0]);
   loadTexture("../media/glass.bmp", GtextureID[1]);
   loadTexture("../media/gradientGray.bmp", GtextureID[2]);
   loadTexture("../media/fury_nano2.bmp", GtextureID[3]);
   loadTexture("../media/gradientBloom.bmp", GtextureID[4]);
   loadTexture("../media/gradientDiamond.bmp", GtextureID[5]);
   // ====

   glGenTextures(1, &GCubeMaptextureID);
   std::vector<std::string> skybox
   {
		"../media/right.bmp",
		"../media/left.bmp",
		"../media/bottom.bmp",
		"../media/top.bmp",
		"../media/front.bmp",
		"../media/back.bmp"
   };
   loadCubemapTexture(skybox, GCubeMaptextureID);

   genPlane();

   // ================ Set up frame buffer, render buffer, and create an empty texture for blurring purpose
   // Create a new FBO
   glGenFramebuffers(1, &Gframebuffer);

   // Create a new empty texture to render original scene
   glGenTextures(1, &GfullscreenTextureMain);
   glBindTexture(GL_TEXTURE_2D, GfullscreenTextureMain);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   // 2nd texture to render high pass filter
   glGenTextures(1, &GfullscreenTextureHighpass);
   glBindTexture(GL_TEXTURE_2D, GfullscreenTextureHighpass);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   // 3rd texture to render 1st blur
   glGenTextures(1, &GfullscreenTextureBlur1);
   glBindTexture(GL_TEXTURE_2D, GfullscreenTextureBlur1);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   // 4th texture to render 2nd blur
   glGenTextures(1, &GfullscreenTextureBlur2);
   glBindTexture(GL_TEXTURE_2D, GfullscreenTextureBlur2);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   // 5th texture to render grayscale
   glGenTextures(1, &GfullscreenTextureGray);
   glBindTexture(GL_TEXTURE_2D, GfullscreenTextureGray);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   // Create and bind renderbuffer, and create a 16-bit depth buffer
   glGenRenderbuffers(1, &GdepthRenderbuffer);
   glBindRenderbuffer(GL_RENDERBUFFER, GdepthRenderbuffer);
   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, WINDOW_WIDTH, WINDOW_HEIGHT);
   // ====================


   // ==================== 1st shader set up ====================
   fragmentShader1 = LoadShaderFromFile(GL_VERTEX_SHADER, "../vertexShader1.vert" );
   vertexShader1 = LoadShaderFromFile(GL_FRAGMENT_SHADER, "../fragmentShader1.frag" );

   // Create the program object
   programObject1 = glCreateProgram ( );
   
   if ( programObject1 == 0 )
      return 0;

   glAttachShader ( programObject1, vertexShader1 );
   glAttachShader ( programObject1, fragmentShader1 );

   glBindAttribLocation ( programObject1, 0, "vPosition" );
   glBindAttribLocation ( programObject1, 1, "vTexCoord" );

   // Link the program
   glLinkProgram ( programObject1 );

   // Check the link status
   glGetProgramiv ( programObject1, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;
      glGetProgramiv ( programObject1, GL_INFO_LOG_LENGTH, &infoLen );
      if ( infoLen > 1 )
      {
		 char infoLog[1024];
         glGetProgramInfoLog ( programObject1, infoLen, NULL, infoLog );
         printf ( "Error linking program:\n%s\n", infoLog );            
      }

      glDeleteProgram ( programObject1 );
      return 0;
   }

   // Store the program object
   GprogramID1 = programObject1;
   // ====================

   // ==================== (Twist) 2nd shader set up ====================
   fragmentShader2 = LoadShaderFromFile(GL_VERTEX_SHADER, "../vertexShaderTwist.vert");
   vertexShader2 = LoadShaderFromFile(GL_FRAGMENT_SHADER, "../twistFragmentShader.frag");

   // Create the program object
   programObject2 = glCreateProgram();

   if (programObject2 == 0)
	   return 0;

   glAttachShader(programObject2, vertexShader2);
   glAttachShader(programObject2, fragmentShader2);

   glBindAttribLocation(programObject2, 0, "vPosition");
   glBindAttribLocation(programObject2, 1, "vTexCoord");

   // Link the program
   glLinkProgram(programObject2);

   // Check the link status
   glGetProgramiv(programObject2, GL_LINK_STATUS, &linked);

   if (!linked)
   {
	   GLint infoLen = 0;
	   glGetProgramiv(programObject2, GL_INFO_LOG_LENGTH, &infoLen);
	   if (infoLen > 1)
	   {
		   char infoLog[1024];
		   glGetProgramInfoLog(programObject2, infoLen, NULL, infoLog);
		   printf("Error linking program:\n%s\n", infoLog);
	   }

	   glDeleteProgram(programObject2);
	   return 0;
   }

   // Store the program object
   GTwistProgramID = programObject2;
   // ====================

	// ==================== (Skybox) 3rd shader set up ====================
   vertexShader3 = LoadShaderFromFile(GL_VERTEX_SHADER, "../skyboxVertexShader.vert");
   fragmentShader3 = LoadShaderFromFile(GL_FRAGMENT_SHADER, "../skyboxFragmentShader.frag");

   // Create the program object
   programObject3 = glCreateProgram();

   if (programObject3 == 0)
	   return 0;

   glAttachShader (programObject3, vertexShader3);
   glAttachShader (programObject3, fragmentShader3);

   glBindAttribLocation (programObject3, 0, "vPosition");

   // Link the program
   glLinkProgram(programObject3);

   // Check the link status
   glGetProgramiv(programObject3, GL_LINK_STATUS, &linked);

   if (!linked)
   {
	   GLint infoLen = 0;
	   glGetProgramiv (programObject3, GL_INFO_LOG_LENGTH, &infoLen);
	   if (infoLen > 1)
	   {
		   char infoLog[1024];
		   glGetProgramInfoLog (programObject3, infoLen, NULL, infoLog);
		   printf ("Error linking program:\n%s\n", infoLog);
	   }

	   glDeleteProgram (programObject3);
	   return 0;
   }

   GSkyBoxProgramID = programObject3;
   // ===============================


   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


   // Initialize matrices
   gPerspectiveMatrix = Matrix4::perspective(60.0f,
											(float)WINDOW_WIDTH/(float)WINDOW_HEIGHT,
                                             0.5f, 400.0f);
  // gOthorMatrix = Matrix4::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.5f, 30.0f);

   gViewMatrix = Matrix4::translate(Vector3(0.0f, 0.0f, -15.0f));


   return 1;
}


void UpdateCamera(void)
{
	if(glfwGetKey(window, 'A')) cameraPitch -= 1.0f;
	if(glfwGetKey(window, 'D')) cameraPitch += 1.0f;
	if(glfwGetKey(window, 'W')) cameraYaw -= 1.0f;
	if(glfwGetKey(window, 'S')) cameraYaw += 1.0f;

	if(glfwGetKey(window, 'R'))
	{
		cameraDistance -= 0.06f;
		if(cameraDistance < 1.0f)
			cameraDistance = 1.0f;
	}
	if(glfwGetKey(window, 'F')) cameraDistance += 0.06f;

	gViewMatrix = Matrix4::translate(Vector3(0.0f, 0.0f, -cameraDistance)) *
				  Matrix4::rotate(cameraYaw, Vector3(1.0f, 0.0f, 0.0f)) *
				  Matrix4::rotate(cameraPitch, Vector3(0.0f, 1.0f, 0.0f));

	/*if (glfwGetKey(window, 'Q') && isGrayOut == true) isGrayOut = false;
	else if (glfwGetKey(window, 'Q') && isGrayOut == false) isGrayOut = true;

	if (glfwGetKey(window, 'E') && isBloomOut == true) isBloomOut = false;
	else if (glfwGetKey(window, 'E') && isBloomOut == false) isBloomOut = true;*/

	if (glfwGetKey(window, '1')) highPassLimit -= 0.005;
	if (glfwGetKey(window, '2')) highPassLimit += 0.005;
}

void ToggleEffects(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		isBloomOut = !isBloomOut;
		/*if (isGrayOut)
			isGrayOut = false;*/
	}
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		isGrayOut = !isGrayOut;
		/*if (isBloomOut)
			isBloomOut = false;*/
	}
}

Matrix4 getViewMatrixWithoutTranslate(void)
{
	return Matrix4::rotate(cameraYaw, Vector3(1.0f, 0.0f, 0.0f)) * 
		   Matrix4::rotate(cameraPitch, Vector3(0.0f, 1.0f, 0.0f));
}

void DrawSquare(GLuint texture)
{
    static GLfloat vVertices[] = { -1.0f,  1.0f, 0.0f,
								   -1.0f, -1.0f, 0.0f,
								    1.0f, -1.0f, 0.0f,
								    1.0f, -1.0f, 0.0f,
								    1.0f,  1.0f, 0.0f,
								   -1.0f,  1.0f, 0.0f };
					

   static GLfloat vTexCoords[] = { 0.0f, 1.0f,
								   0.0f, 0.0f,
								   1.0f, 0.0f,
								   1.0f, 0.0f,
								   1.0f, 1.0f,
								   0.0f, 1.0f };

   glBindTexture(GL_TEXTURE_2D, texture);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, vTexCoords);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);

   glDrawArrays(GL_TRIANGLES, 0, 6);

   glDisableVertexAttribArray(0);
   glDisableVertexAttribArray(1);
}

void DrawCube(GLuint texture)
{
	static GLfloat vVertices[] = { -1.0f,  1.0f, 1.0f,
								   -1.0f, -1.0f, 1.0f,
									1.0f, -1.0f, 1.0f,
									1.0f, -1.0f, 1.0f,
									1.0f,  1.0f, 1.0f,
								   -1.0f,  1.0f, 1.0f,
	
							   	   -1.0f,  1.0f, -1.0f,
								   -1.0f, -1.0f, -1.0f,
									1.0f, -1.0f, -1.0f,
									1.0f, -1.0f, -1.0f,
									1.0f,  1.0f, -1.0f,
								   -1.0f,  1.0f, -1.0f,
									// = = =
								    1.0f,  1.0f,  1.0f,
								    1.0f, -1.0f,  1.0f,
									1.0f, -1.0f, -1.0f,
									1.0f, -1.0f, -1.0f,
									1.0f,  1.0f, -1.0f,
								    1.0f,  1.0f,  1.0f,

									-1.0f,  1.0f,  1.0f,
									-1.0f, -1.0f,  1.0f,
									-1.0f, -1.0f, -1.0f,
									-1.0f, -1.0f, -1.0f,
									-1.0f,  1.0f, -1.0f,
									-1.0f,  1.0f,  1.0f,
									// = = =
									-1.0f, 1.0f, -1.0f,
									-1.0f, 1.0f,  1.0f,
									 1.0f, 1.0f,  1.0f,
									 1.0f, 1.0f,  1.0f,
									 1.0f, 1.0f, -1.0f,
									-1.0f, 1.0f, -1.0f,

									-1.0f, -1.0f, -1.0f,
									-1.0f, -1.0f,  1.0f,
									 1.0f, -1.0f,  1.0f,
									 1.0f, -1.0f,  1.0f,
									 1.0f, -1.0f, -1.0f,
									-1.0f, -1.0f, -1.0f };

	static GLfloat vTexCoords[] = { 0.0f, 1.0f,
									0.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 1.0f,
									0.0f, 1.0f,

									0.0f, 1.0f,
									0.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 1.0f,
									0.0f, 1.0f,
									// • • •
									0.0f, 1.0f,
									0.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 1.0f,
									0.0f, 1.0f,

									0.0f, 1.0f,
									0.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 1.0f,
									0.0f, 1.0f,
									// • • •
									0.0f, 1.0f,
									0.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 1.0f,
									0.0f, 1.0f,

									0.0f, 1.0f,
									0.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 0.0f,
									1.0f, 1.0f,
									0.0f, 1.0f };

	glBindTexture(GL_TEXTURE_2D, texture);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, vTexCoords);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 36); // Set to 24 to open top else 36

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void Draw(void)
{	
	rotate += 0.05;

	Matrix4 modelMatrix, mvpMatrix;

	// Use the program object, it's possible that you have multiple shader programs and switch it accordingly
	//glUseProgram(GSkyBoxProgramID);

	// Set the sampler2D varying variable to the first texture unit(index 0)
	glUniform1i(glGetUniformLocation(GprogramID1, "sampler2d"), 0);

	// ======== Pass texture size to shader
	glUniform1f(glGetUniformLocation(GprogramID1, "uTextureW"), (GLfloat)WINDOW_WIDTH);
	glUniform1f(glGetUniformLocation(GprogramID1, "uTextureH"), (GLfloat)WINDOW_HEIGHT);
	// ========

	UpdateCamera();
	
	// Set the viewport
	glViewport(0, 0, GtexBGWidth, GtexBGHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, Gframebuffer);

	// Specify texture as color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GfullscreenTextureMain, 0);

	// Specify depth_renderbuffer as depth attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, GdepthRenderbuffer);

	// • • • Drawing main objects in scene • • •
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniform1i(glGetUniformLocation(GprogramID1, "uHighPass"), -1); // Do nothing
		glUniform1i(glGetUniformLocation(GprogramID1, "uBlurDirection"), -2); // Set to no blur & draw normal
		glUniform1i(glGetUniformLocation(GprogramID1, "uGray"), -1); // Don't grayscale

		glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

		// Render skybox
		glUseProgram(GSkyBoxProgramID);

		glUniform1i(glGetUniformLocation(GSkyBoxProgramID, "samplerCube1"), 0);
		modelMatrix = Matrix4::scale(Vector3(50.0f, 50.0f, 50.0f));
		mvpMatrix = gPerspectiveMatrix * getViewMatrixWithoutTranslate() * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GSkyBoxProgramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		drawSkyBox();
		// ========

		// Use 1st shader
		glUseProgram(GprogramID1);

		// First object
		static float rotationAngle = 0.0;
		if (isBloomOut) rotationAngle = 45;
		else rotationAngle = -45;

		modelMatrix =
			Matrix4::translate(Vector3(4.0f, 0.0f, -4.0f)) *
			Matrix4::scale(Vector3(3.0f, 3.0f, 3.0f)) *
			Matrix4::rotate(rotationAngle, Vector3(0.0f, 1.0f, 0.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[2]);
		// ========

		// 2nd object
		modelMatrix =
			Matrix4::translate(Vector3(-4.0f, 0.0f, -4.0f)) *
			Matrix4::scale(Vector3(3.0f, 3.0f, 3.0f)) *
			Matrix4::rotate(rotationAngle, Vector3(0.0f, 1.0f, 0.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawSquare(GtextureID[4]);

		// 3rd object
		modelMatrix =
			Matrix4::translate(Vector3(0.0f, 8.0f, -7.0f)) *
			Matrix4::scale(Vector3(1.0f, 1.0f, 1.0f)) *
			Matrix4::rotate(sin(rotate) * 100.0, Vector3(0.0f, 1.0f, 0.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawCube(GtextureID[5]);

		// 4th object
		modelMatrix =
			Matrix4::translate(Vector3(0.0f, highPassLimit * 4.0, -14.0f)) *
			Matrix4::scale(Vector3(0.8f, 1.7f, 0.8f)) *
			Matrix4::rotate(rotate * 10.0, Vector3(0.0f, 1.0f, 0.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawCube(GtextureID[5]);

		// 5th object
		modelMatrix =
			Matrix4::translate(Vector3(0.0f, -8.0 + sin(rotate), -5.0f)) *
			Matrix4::scale(Vector3(1.0f, 1.0f, 1.0f)) *
			Matrix4::rotate(rotate * 10.0, Vector3(1.0f, 1.0f, 1.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawCube(GtextureID[5]);

		// Use twist vertex program
		glUseProgram(GTwistProgramID);

		// Modify Factor 1 varying variable
		static float factor1 = 0.0f;
		factor1 += 0.05f;
		GLint factor1Loc = glGetUniformLocation(GTwistProgramID, "Factor1");
		if (factor1Loc != -1)
		{
			glUniform1f(factor1Loc, cos(factor1));
		}
		// ====================

		// Cube object
		modelMatrix =
			Matrix4::rotate(factor1 * 2.0, Vector3(0.0f, -1.0f, 0.0f)) *
			Matrix4::translate(Vector3(0.0f, 0.0f, -21.0f)) *
			Matrix4::scale(Vector3(1.0f, 3.0f, 1.0f));
		mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(GTwistProgramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
		DrawCube(GtextureID[5]);

		// Reset program
		glUseProgram(GprogramID1);

		if (isGrayOut)
		{
			// • • • Grayscale • • •
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GfullscreenTextureGray, 0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

			glUniform1i(glGetUniformLocation(GprogramID1, "uHighPass"), -1); // Do nothing
			glUniform1i(glGetUniformLocation(GprogramID1, "uBlurDirection"), -1); // Set to no blur & escape
			glUniform1i(glGetUniformLocation(GprogramID1, "uGray"), 0); // Grayscale

			DrawSquare(GfullscreenTextureMain);
			// = = = = = = = =
		}
		
		if (isBloomOut)
		{
			// Pass in value
			GLint highPassLimitLoc = glGetUniformLocation(GprogramID1, "highPassLimit");
			if (highPassLimitLoc != -1)
			{
				glUniform1f(highPassLimitLoc, cos(highPassLimit));
			}
			// = = = =

			// • • • High pass filter • • •
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GfullscreenTextureHighpass, 0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

			glUniform1i(glGetUniformLocation(GprogramID1, "uHighPass"), 0); // High pass filter
			glUniform1i(glGetUniformLocation(GprogramID1, "uBlurDirection"), -1); // Set to no blur & escape
			glUniform1i(glGetUniformLocation(GprogramID1, "uGray"), -1); // Don't grayscale

			if (isGrayOut) DrawSquare(GfullscreenTextureGray);
			else DrawSquare(GfullscreenTextureMain);
			// = = = = = = = =

			// • • • First blur • • •
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GfullscreenTextureBlur1, 0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

			glUniform1i(glGetUniformLocation(GprogramID1, "uHighPass"), -1); // Do nothing
			glUniform1i(glGetUniformLocation(GprogramID1, "uBlurDirection"), 0); // Horizontal blur

			DrawSquare(GfullscreenTextureHighpass);
			// = = = = = = = =

			// • • • Second blur • • •
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GfullscreenTextureBlur2, 0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

			glUniform1i(glGetUniformLocation(GprogramID1, "uBlurDirection"), 1); // Vertical blur

			DrawSquare(GfullscreenTextureBlur1);
			// = = = = = = = =
		}
	}
	else
	{
		printf("Frame buffer is not ready! Line: 537\n");
	}
	// = = = = = = = =

	if (isBloomOut)
	{
		// Set active texture
		glActiveTexture(GL_TEXTURE0 + 1);
		if (isBloomOut) glBindTexture(GL_TEXTURE_2D, GfullscreenTextureBlur2);
		glUniform1i(glGetUniformLocation(GprogramID1, "bloomTexture"), 1);

		glActiveTexture(GL_TEXTURE0);
		// = = = = = = = =
	}

	// This time, render directly to window system framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset the mvpMatrix to identity matrix so that it renders fully on texture in normalized device coordinates
	glUniformMatrix4fv(glGetUniformLocation(GprogramID1, "uMvpMatrix"), 1, GL_FALSE, Matrix4::identity().data);

	// Draw the texture that has been screen captured
	if (isBloomOut) glUniform1i(glGetUniformLocation(GprogramID1, "uBlurDirection"), 2); // Set to no blur & add bloom
	else glUniform1i(glGetUniformLocation(GprogramID1, "uBlurDirection"), -2); // Set to no blur & draw normal

	if (isGrayOut) DrawSquare(GfullscreenTextureGray);
	else DrawSquare(GfullscreenTextureMain);
	// ===================================================
}

int main(void)
{
  glfwSetErrorCallback(error_callback);

  // Initialize GLFW library
  if (!glfwInit())
    return -1;

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create and open a window
  window = glfwCreateWindow(WINDOW_WIDTH,
                            WINDOW_HEIGHT,
                            "Hello World",
                            NULL,
                            NULL);

  if(!window)
  {
    glfwTerminate();
    printf("glfwCreateWindow Error\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);

  glfwSwapInterval(1);
  Init();

  // Repeat
  while(!glfwWindowShouldClose(window))
  {
	  Draw();
	  glfwSwapBuffers(window);
	  glfwPollEvents();

	  // Toggle post processing
	  glfwSetKeyCallback(window, ToggleEffects);

	  if(glfwGetKey(window, GLFW_KEY_ESCAPE))
			break;
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
