#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <GL/glew.h>
#include <include/GL/gl.h>
#include <include/GL/glext.h>
#include <include/GL/glut.h>
#include <include/GL/glm/glm.hpp>
#include <include/GL/glm/gtc/matrix_transform.hpp>
#include <include/GL/glm/gtx/transform2.hpp>
#include <include/GL/glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

// #include <png.h>
// #define TEXTURE_LOAD_ERROR 0


#define GL_ERROR() checkForOpenGLError(__FILE__, __LINE__)
using namespace std;
using glm::mat4;
using glm::vec3;

GLuint g_vao;
GLuint g_programHandle;
GLuint g_winWidth = 800;
GLuint g_winHeight = 800;
GLint g_angle = 0;
GLuint g_frameBuffer;
// transfer function
GLuint g_bfTexObj;
GLuint g_texWidth;
GLuint g_texHeight;
GLuint g_rcVertHandle;
GLuint g_rcFragHandle;
GLuint g_bfVertHandle;
GLuint g_bfFragHandle;
GLuint pngTex;
GLuint trTex;

float g_stepSize = 256.0;
float g_NumberOfSlices = 255.0;
float g_MinGrayVal = 0.001; // 0
float g_MaxGrayVal = 1.0; // 1
float g_OpacityVal = 4.0; // 40
float g_ColorVal = 1.0; // 0.4
float g_AbsorptionModeIndex = 1.0; // -1.0 ? 1
float g_SlicesOverX = 16.0; // 16
float g_SlicesOverY = 16.0; // 16

int tr_width = 256;
int tr_height = 10;

int png_width = 4096;
int png_height = 4096;


int checkForOpenGLError(const char* file, int line)
{
    // return 1 if an OpenGL error occured, 0 otherwise.
    GLenum glErr;
    int retCode = 0;

    glErr = glGetError();
    while(glErr != GL_NO_ERROR)
    {
    	cout << "glError in file " << file
    	     << "@line " << line << gluErrorString(glErr) << endl;
    	retCode = 1;
    	exit(EXIT_FAILURE);
    }
    return retCode;
}
void keyboard(unsigned char key, int x, int y);
void display(void);
void initVBO();
void initShader();
void initFrameBuffer(GLuint, GLuint, GLuint);
GLuint initFace2DTex(GLuint texWidth, GLuint texHeight);

void loadImage(const char* pathToFile, GLuint* texture, int* width, int* height)
{
  unsigned char* tempTexture = SOIL_load_image(pathToFile, width, height, 0, SOIL_LOAD_RGBA);

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_2D, *texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempTexture);
  glGenerateMipmap(GL_TEXTURE_2D);

  SOIL_free_image_data(tempTexture);
}


void render(GLenum cullFace);
void init()
{
    g_texWidth = g_winWidth;
    g_texHeight = g_winHeight;
    initVBO();
    initShader();


    loadImage("../bonsai.raw.png", &pngTex, &png_width, &png_height);
    loadImage("../cm_BrBG_r.png", &trTex, &tr_width, &tr_height);

    g_bfTexObj = initFace2DTex(g_texWidth, g_texHeight);
    GL_ERROR();

    initFrameBuffer(g_bfTexObj, g_texWidth, g_texHeight);
    GL_ERROR();
}
// init the vertex buffer object
void initVBO()
{
    GLfloat vertices[24] = {
    	0.0, 0.0, 0.0,
    	0.0, 0.0, 1.0,
    	0.0, 1.0, 0.0,
    	0.0, 1.0, 1.0,
    	1.0, 0.0, 0.0,
    	1.0, 0.0, 1.0,
    	1.0, 1.0, 0.0,
    	1.0, 1.0, 1.0
    };

    GLuint indices[36] = {
    	1,5,7,
    	7,3,1,
    	0,2,6,
      6,4,0,
    	0,1,3,
    	3,2,0,
    	7,5,4,
    	4,6,7,
    	2,3,7,
    	7,6,2,
    	1,0,4,
    	4,5,1
    };
    GLuint gbo[2];

    glGenBuffers(2, gbo);
    GLuint vertexdat = gbo[0];
    GLuint veridxdat = gbo[1];
    glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // used in glDrawElement()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), indices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    // vao like a closure binding 3 buffer object: verlocdat vercoldat and veridxdat
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0); // for vertexloc
    glEnableVertexAttribArray(1); // for vertexcol

    // the vertex location is the same as the vertex color
    glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
    // glBindVertexArray(0);
    g_vao = vao;
}
void drawBox(GLenum glFaces)
{
    glEnable(GL_CULL_FACE);
    glCullFace(glFaces);
    glBindVertexArray(g_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
    glDisable(GL_CULL_FACE);
}
// check the compilation result
GLboolean compileCheck(GLuint shader)
{
    GLint err;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &err);
    if (GL_FALSE == err)
    {
    	GLint logLen;
    	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
    	if (logLen > 0)
    	{
  	    char* log = (char *)malloc(logLen);
  	    GLsizei written;
  	    glGetShaderInfoLog(shader, logLen, &written, log);
  	    cerr << "Shader log: " << log << endl;
  	    free(log);
    	}
    }
    return err;
}
// init shader object
GLuint initShaderObj(const GLchar* srcfile, GLenum shaderType)
{
    ifstream inFile(srcfile, ifstream::in);
    // use assert?
    if (!inFile)
    {
    	cerr << "Error openning file: " << srcfile << endl;
    	exit(EXIT_FAILURE);
    }

    const int MAX_CNT = 10000;
    GLchar *shaderCode = (GLchar *) calloc(MAX_CNT, sizeof(GLchar));
    inFile.read(shaderCode, MAX_CNT);
    if (inFile.eof())
    {
    	size_t bytecnt = inFile.gcount();
    	*(shaderCode + bytecnt) = '\0';
    }
    else if(inFile.fail()) cout << srcfile << "read failed \n";
    else cout << srcfile << "is too large\n";
    // create the shader Object
    GLuint shader = glCreateShader(shaderType);
    if (0 == shader) cerr << "Error creating vertex shader.\n";
    const GLchar* codeArray[] = {shaderCode};
    glShaderSource(shader, 1, codeArray, NULL);
    free(shaderCode);

    // compile the shader
    glCompileShader(shader);
    if (GL_FALSE == compileCheck(shader)) cerr << "shader compilation failed\n";
    return shader;
}
GLint checkShaderLinkStatus(GLuint pgmHandle)
{
    GLint status;
    glGetProgramiv(pgmHandle, GL_LINK_STATUS, &status);
    if (GL_FALSE == status)
    {
    	GLint logLen;
    	glGetProgramiv(pgmHandle, GL_INFO_LOG_LENGTH, &logLen);
    	if (logLen > 0)
    	{
    	    GLchar * log = (GLchar *)malloc(logLen);
    	    GLsizei written;
    	    glGetProgramInfoLog(pgmHandle, logLen, &written, log);
    	    cerr << "Program log: " << log << endl;
    	}
    }
    return status;
}
// link shader program
GLuint createShaderPgm()
{
    // Create the shader program
    GLuint programHandle = glCreateProgram();
    if (0 == programHandle)
    {
    	cerr << "Error create shader program" << endl;
    	exit(EXIT_FAILURE);
    }
    return programHandle;
}

GLuint initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight)
{
    GLuint backFace2DTex;
    glGenTextures(1, &backFace2DTex);
    glBindTexture(GL_TEXTURE_2D, backFace2DTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bfTexWidth, bfTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    return backFace2DTex;
}

void checkFramebufferStatus()
{
    GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (complete != GL_FRAMEBUFFER_COMPLETE)
    {
    	cout << "framebuffer is not complete" << endl;
    	exit(EXIT_FAILURE);
    }
}
// init the framebuffer, the only framebuffer used in this program
void initFrameBuffer(GLuint texObj, GLuint texWidth, GLuint texHeight)
{
    // create a depth buffer for our framebuffer
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);

    // attach the texture and the depth buffer to the framebuffer
    glGenFramebuffers(1, &g_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, g_frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    checkFramebufferStatus();
    glEnable(GL_DEPTH_TEST);
}

void rcSetUinforms()
{
    GLint stepSizeLoc = glGetUniformLocation(g_programHandle, "uSteps");
    GL_ERROR();
    if (stepSizeLoc >= 0) glUniform1f(stepSizeLoc, g_stepSize);
    else cout << "uSteps is not bind to the uniform\n";
    GL_ERROR();
    GLint transferFuncLoc = glGetUniformLocation(g_programHandle, "uTransferFunction");
    if (transferFuncLoc >= 0)
    {
      	glActiveTexture(GL_TEXTURE0);
      	glBindTexture(GL_TEXTURE_2D, trTex);
      	glUniform1i(transferFuncLoc, 0);
    }
    else cout << "uBackCoord is not bind to the uniform\n";
    GL_ERROR();
    GLint backFaceLoc = glGetUniformLocation(g_programHandle, "uBackCoord");
    if (backFaceLoc >= 0)
    {
      	glActiveTexture(GL_TEXTURE1);
      	glBindTexture(GL_TEXTURE_2D, g_bfTexObj);
      	glUniform1i(backFaceLoc, 1);
    }
    else cout << "uBackCoord is not bind to the uniform\n";
    GL_ERROR();
    GLint volumeLoc = glGetUniformLocation(g_programHandle, "uSliceMaps");
    if (volumeLoc >= 0)
    {
    	glActiveTexture(GL_TEXTURE2);
    	glBindTexture(GL_TEXTURE_2D, pngTex);
    	glUniform1i(volumeLoc, 2);
    }
    else cout << "uSliceMaps is not bind to the uniform\n";


    GLint uNumberOfSlicesLoc = glGetUniformLocation(g_programHandle, "uNumberOfSlices");
    if (uNumberOfSlicesLoc >= 0) glUniform1f(uNumberOfSlicesLoc, g_NumberOfSlices);
    else cout << "uNumberOfSlices is not bind to the uniform\n";

    GLint uMinGrayValLoc = glGetUniformLocation(g_programHandle, "uMinGrayVal");
    if (uMinGrayValLoc >= 0) glUniform1f(uMinGrayValLoc, g_MinGrayVal);
    else cout << "uMinGrayVal is not bind to the uniform\n";

    GLint uMaxGrayValLoc = glGetUniformLocation(g_programHandle, "uMaxGrayVal");
    if (uMaxGrayValLoc >= 0) glUniform1f(uMaxGrayValLoc, g_MaxGrayVal);
    else cout << "uMaxGrayVal is not bind to the uniform\n";

    GLint uOpacityValLoc = glGetUniformLocation(g_programHandle, "uOpacityVal");
    if (uOpacityValLoc >= 0) glUniform1f(uOpacityValLoc, g_OpacityVal);
    else cout << "uOpacityVal is not bind to the uniform\n";

    GLint uColorValLoc = glGetUniformLocation(g_programHandle, "uColorVal");
    if (uColorValLoc >= 0) glUniform1f(uColorValLoc, g_ColorVal);
    else cout << "uColorVal is not bind to the uniform\n";

    GLint uAbsorptionModeIndexLoc = glGetUniformLocation(g_programHandle, "uAbsorptionModeIndex");
    if (uAbsorptionModeIndexLoc >= 0) glUniform1f(uAbsorptionModeIndexLoc, g_AbsorptionModeIndex);
    else cout << "uAbsorptionModeIndex is not bind to the uniform\n";

    GLint uSlicesOverXLoc = glGetUniformLocation(g_programHandle, "uSlicesOverX");
    if (uSlicesOverXLoc >= 0) glUniform1f(uSlicesOverXLoc, g_SlicesOverX);
    else cout << "uSlicesOverX is not bind to the uniform\n";

    GLint uSlicesOverYLoc = glGetUniformLocation(g_programHandle, "uSlicesOverY");
    if (uSlicesOverYLoc >= 0) glUniform1f(uSlicesOverYLoc, g_SlicesOverY);
    else cout << "uSlicesOverY is not bind to the uniform\n";

}
// init the shader object and shader program
void initShader()
{
// vertex shader object for first pass
    g_bfVertHandle = initShaderObj("../shader/firstPass.vert", GL_VERTEX_SHADER);
// fragment shader object for first pass
    g_bfFragHandle = initShaderObj("../shader/firstPass.frag", GL_FRAGMENT_SHADER);
// vertex shader object for second pass
    g_rcVertHandle = initShaderObj("../shader/secondPass.vert", GL_VERTEX_SHADER);
// fragment shader object for second pass
    g_rcFragHandle = initShaderObj("../shader/secondPass.frag", GL_FRAGMENT_SHADER);
// create the shader program , use it in an appropriate time
    g_programHandle = createShaderPgm();
}

// link the shader objects using the shader program
void linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle)
{
    const GLsizei maxCount = 2;
    GLsizei count;
    GLuint shaders[maxCount];
    glGetAttachedShaders(shaderPgm, maxCount, &count, shaders);

    GL_ERROR();
    for (int i = 0; i < count; i++) {
	     glDetachShader(shaderPgm, shaders[i]);
    }
    // Bind index 0 to the shader input variable "VerPos"
    glBindAttribLocation(shaderPgm, 0, "position");
    // Bind index 1 to the shader input variable "VerClr"
    glBindAttribLocation(shaderPgm, 1, "vertColor");
    GL_ERROR();
    glAttachShader(shaderPgm,newVertHandle);
    glAttachShader(shaderPgm,newFragHandle);
    GL_ERROR();
    glLinkProgram(shaderPgm);
    if (GL_FALSE == checkShaderLinkStatus(shaderPgm))
    {
    	cerr << "Failed to relink shader program!" << endl;
    	exit(EXIT_FAILURE);
    }
    GL_ERROR();
}

void display()
{
    glEnable(GL_DEPTH_TEST);
    GL_ERROR();
    // render to texture
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_frameBuffer);
    glViewport(0, 0, g_winWidth, g_winHeight);
    linkShader(g_programHandle, g_bfVertHandle, g_bfFragHandle);
    glUseProgram(g_programHandle);
    // cull front face
    render(GL_FRONT);
    glUseProgram(0);
    GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, g_winWidth, g_winHeight);
    linkShader(g_programHandle, g_rcVertHandle, g_rcFragHandle);
    GL_ERROR();
    glUseProgram(g_programHandle);
    rcSetUinforms();
    GL_ERROR();
    render(GL_BACK);
    glUseProgram(0);
    GL_ERROR();

    glutSwapBuffers();
}
void render(GLenum cullFace)
{
    GL_ERROR();
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //  transform the box
    glm::mat4 projection = glm::perspective(60.0f, (GLfloat)g_winWidth/g_winHeight, 0.1f, 400.f);
    glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 1.0f, 2.0f),
    				 glm::vec3(0.0f, 0.0f, 0.0f),
    				 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = mat4(1.0f);
    model *= glm::rotate((float)g_angle, glm::vec3(0.0f, 1.0f, 0.0f));
    // to make the "head256.raw" i.e. the volume data stand up.
    model *= glm::rotate(180.0f, vec3(1.0f, 0.0f, 0.0f));
    model *= glm::translate(glm::vec3(-0.5f, -0.5f, -0.5f));
    // notice the multiplication order: reverse order of transform
    glm::mat4 mvp = projection * view * model;
    GLuint mvpIdx = glGetUniformLocation(g_programHandle, "MVP");
    if (mvpIdx >= 0)
    {
    	glUniformMatrix4fv(mvpIdx, 1, GL_FALSE, &mvp[0][0]);
    }
    else
    {
    	cerr << "can't get the MVP" << endl;
    }
    GL_ERROR();
    drawBox(cullFace);
    GL_ERROR();
}
void rotateDisplay()
{
    g_angle = (g_angle + 1) % 360;
    glutPostRedisplay();
}
void reshape(int w, int h)
{
    g_winWidth = w;
    g_winHeight = h;
    g_texWidth = w;
    g_texHeight = h;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '\x1B':
	exit(EXIT_SUCCESS);
	break;
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("GLUT Test");
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
    	/* Problem: glewInit failed, something is seriously wrong. */
    	fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    glutKeyboardFunc(&keyboard);
    glutDisplayFunc(&display);
    glutReshapeFunc(&reshape);
    glutIdleFunc(&rotateDisplay);
    init();
    glutMainLoop();
    return EXIT_SUCCESS;
}
