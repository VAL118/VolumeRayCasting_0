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
GLuint g_winWidth = 400;
GLuint g_winHeight = 400;
GLint g_angle = 0;
GLuint g_frameBuffer;
// transfer function
GLuint g_tffTexObj;
GLuint g_bfTexObj;
GLuint g_texWidth;
GLuint g_texHeight;
GLuint g_volTexObj;
GLuint g_rcVertHandle;
GLuint g_rcFragHandle;
GLuint g_bfVertHandle;
GLuint g_bfFragHandle;
GLuint pngTex;
GLuint trTex;

float g_stepSize = 256.0;
float g_NumberOfSlices = 255.0;
float g_MinGrayVal = 0.0; // 0
float g_MaxGrayVal = 1.0; // 1
float g_OpacityVal = 0.5; // 40
float g_ColorVal = 0.4; // 0.4
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
GLuint initPNG2DTex(const char* filename, GLuint width, GLuint height);
GLuint initFace2DTex(GLuint texWidth, GLuint texHeight);
GLuint initVol2DTex(const char* filename, GLuint width, GLuint height);

// GLuint loadTexture(const string filename, int &width, int &height);

void render(GLenum cullFace);
void init()
{
    g_texWidth = g_winWidth;
    g_texHeight = g_winHeight;
    initVBO();
    initShader();
    // g_tffTexObj = initPNG2DTex("../cm_BrBG_r.png", 256, 10);


    unsigned char* tempTexture0 = SOIL_load_image("../cm_BrBG_r.png", &tr_width, &tr_height, 0, SOIL_LOAD_RGBA);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &trTex);
    glBindTexture(GL_TEXTURE_2D,trTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tr_width, tr_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempTexture0);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(tempTexture0);



    /*int width = 256;
    int height = 10;
    g_tffTexObj = loadTexture("../cm_BrBG_r.png", &width, &height);*/

    g_bfTexObj = initFace2DTex(g_texWidth, g_texHeight);
    // g_volTexObj = initVol2DTex("../bonsai.raw", 4096, 4096);
    GL_ERROR();


    unsigned char* tempTexture = SOIL_load_image("../bonsai.raw.png", &png_width, &png_height, 0, SOIL_LOAD_RGBA);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &pngTex);
    glBindTexture(GL_TEXTURE_2D,pngTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, png_width, png_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempTexture);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(tempTexture);

    // GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");



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


// init the 1 dimentional texture for transfer function
GLuint initPNG2DTex(const char* filename, GLuint w, GLuint h)
{
  FILE *fp;
  size_t size = w * h;
  GLubyte *data = new GLubyte[size];			  // 8bit
  if (!(fp = fopen(filename, "rb")))
  {
      cout << "Error: opening .png file failed 1\n";
      exit(EXIT_FAILURE);
  }
  else cout << "OK: open .png file successed\n";
  if ( size_t sss = fread(data, sizeof(char), size, fp)!= size)
  {
      cout << "Error: read .png file failed 2\n";
      // exit(1);
  }
  else cout << "OK: read .png file successed\n";

  fclose(fp);

  GLuint pngTex;
  glGenTextures(1, &pngTex);
  // bind 2D texture target
  // glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  glBindTexture(GL_TEXTURE_2D, pngTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

  delete []data;
  cout << "volume texture created" << endl;
  return pngTex;
}

/*GLuint loadTexture(const string filename, int &width, int &height)
 {
   //header for testing if it is a png
   png_byte header[8];

   //open file as binary
   FILE *fp = fopen(filename.c_str(), "rb");
   if (!fp) {
     return TEXTURE_LOAD_ERROR;
   }

   //read the header
   fread(header, 1, 8, fp);

   //test if png
   int is_png = !png_sig_cmp(header, 0, 8);
   if (!is_png) {
     fclose(fp);
     return TEXTURE_LOAD_ERROR;
   }

   //create png struct
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
       NULL, NULL);
   if (!png_ptr) {
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }

   //create png info struct
   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
     png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }

   //create png info struct
   png_infop end_info = png_create_info_struct(png_ptr);
   if (!end_info) {
     png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }

   //png error stuff, not sure libpng man suggests this.
   if (setjmp(png_jmpbuf(png_ptr))) {
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }

   //init png reading
   png_init_io(png_ptr, fp);

   //let libpng know you already read the first 8 bytes
   png_set_sig_bytes(png_ptr, 8);

   // read all the info up to the image data
   png_read_info(png_ptr, info_ptr);

   //variables to pass to get info
   int bit_depth, color_type;
   png_uint_32 twidth, theight;

   // get info about png
   png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type,
       NULL, NULL, NULL);

   //update width and height based on png info
   width = twidth;
   height = theight;

   // Update the png info struct.
   png_read_update_info(png_ptr, info_ptr);

   // Row size in bytes.
   int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

   // Allocate the image_data as a big block, to be given to opengl
   png_byte *image_data = new png_byte[rowbytes * height];
   if (!image_data) {
     //clean up memory and close stuff
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     fclose(fp);
     return TEXTURE_LOAD_ERROR;
   }

   //row_pointers is for pointing to image_data for reading the png with libpng
   png_bytep *row_pointers = new png_bytep[height];
   if (!row_pointers) {
     //clean up memory and close stuff
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     delete[] image_data;
     fclose(fp);
     return TEXTURE_LOAD_ERROR;
   }
   // set the individual row_pointers to point at the correct offsets of image_data
   for (int i = 0; i < height; ++i)
     row_pointers[height - 1 - i] = image_data + i * rowbytes;

   //read the png into image_data through row_pointers
   png_read_image(png_ptr, row_pointers);

   //Now generate the OpenGL texture object
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA, width, height, 0,
       GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) image_data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   //clean up memory and close stuff
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   delete[] image_data;
   delete[] row_pointers;
   fclose(fp);

   return texture;
 }*/

// init the 2D texture for render backface 'bf' stands for backface
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
// init 3D texture to store the volume data used fo ray casting
GLuint initVol2DTex(const char* filename, GLuint w, GLuint h)
{
    FILE *fp;
    size_t size = w * h;
    GLubyte *data = new GLubyte[size];			  // 8bit
    if (!(fp = fopen(filename, "rb")))
    {
        cout << "Error: opening .raw file failed" << endl;
        exit(EXIT_FAILURE);
    }
    else cout << "OK: open .raw file successed\n";
    if (size_t sss = fread(data, sizeof(char), size, fp) != size)
    {
        cout << "Error: read .raw file failed" << endl;
        exit(1);
    }
    else cout << "OK: read .raw file successed\n";

    fclose(fp);

    glGenTextures(1, &g_volTexObj);
    // bind 2D texture target
    // glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    glBindTexture(GL_TEXTURE_2D, g_volTexObj);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

    delete []data;
    cout << "volume texture created" << endl;
    return g_volTexObj;
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
    // cout << "get VertHandle: " << shaders[0] << endl;
    // cout << "get FragHandle: " << shaders[1] << endl;
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
    glClearColor(0.2f,0.2f,0.2f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //  transform the box
    glm::mat4 projection = glm::perspective(60.0f, (GLfloat)g_winWidth/g_winHeight, 0.1f, 400.f);
    glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 1.0f, 2.0f),
    				 glm::vec3(0.0f, 0.0f, 0.0f),
    				 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = mat4(1.0f);
    model *= glm::rotate((float)g_angle, glm::vec3(0.0f, 1.0f, 0.0f));
    // to make the "head256.raw" i.e. the volume data stand up.
    model *= glm::rotate(0.0f, vec3(1.0f, 0.0f, 0.0f));
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
    glutInitWindowSize(400, 400);
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
