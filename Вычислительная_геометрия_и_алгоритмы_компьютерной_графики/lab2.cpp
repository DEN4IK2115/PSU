// FirstPRG.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
//#include <Windows.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

using namespace glm;
using namespace std;

GLFWwindow *g_window;
GLint sizeV = 100;
GLuint g_shaderProgram;
GLint g_uMVP;
GLint g_uMV;
GLint g_uN;
GLint g_uL;
const int countTextures = 2;

class Model
{
public:
	GLuint vbo;
	GLuint ibo;
	GLuint vao;
	GLsizei indexCount;
};

Model g_model;

GLuint createShader(const GLchar *code, GLenum type)
{
	GLuint result = glCreateShader(type);

	glShaderSource(result, 1, &code, NULL);
	glCompileShader(result);

	GLint compiled;
	glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint infoLen = 0;
		glGetShaderiv(result, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 0)
		{
			char *infoLog = new char[infoLen];
			glGetShaderInfoLog(result, infoLen, NULL, infoLog);
			cout << "Shader compilation error" << endl << infoLog << endl;
		}
		glDeleteShader(result);
		return 0;
	}

	return result;
}

GLuint createProgram(GLuint vsh, GLuint fsh)
{
	GLuint result = glCreateProgram();

	glAttachShader(result, vsh);
	glAttachShader(result, fsh);

	glLinkProgram(result);

	GLint linked;
	glGetProgramiv(result, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		GLint infoLen = 0;
		glGetProgramiv(result, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 0)
		{
			char *infoLog = (char *)alloca(infoLen);
			glGetProgramInfoLog(result, infoLen, NULL, infoLog);
			cout << "Shader program linking error" << endl << infoLog << endl;
		}
		glDeleteProgram(result);
		return 0;
	}

	return result;
}

bool createShaderProgram()
{
	g_shaderProgram = 0;

	const GLchar vsh[] =
		"#version 330\n"
		""
		"layout(location = 0) in vec2 a_pos;"		//входной вектор с x,z
		"layout(location = 1) in vec3 a_color;"		//входной вктор с цветом
		""
		"uniform mat4 u_mv;"						//входная матрица mv
		"uniform mat4 u_mvp;"						//входная матрица mvp
		""
		"out vec3 v_color;"							//выходной вектор с цветом
		"out vec3 v_p;"								//выходной вектор с глобальными координатами
		"out vec3 v_n;"								//выходная вектор нормали
		""
		"void main()"
		"{"
		"   v_color = a_color;"

		"	float f = sin(atan(a_pos[0]+a_pos[1]));"
		"	float g_x = (-a_pos[0]-a_pos[1])*(a_pos[0]+a_pos[1])/pow((a_pos[0]+a_pos[1])*(a_pos[0]+a_pos[1])+1, 3/2)+1/sqrt((a_pos[0]+a_pos[1])*(a_pos[0]+a_pos[0])+1);"
		"	float g_z = (-a_pos[0]-a_pos[1])*(a_pos[0]+a_pos[1])/pow((a_pos[0]+a_pos[1])*(a_pos[0]+a_pos[1])+1, 3/2)+1/sqrt((a_pos[0]+a_pos[1])*(a_pos[0]+a_pos[0])+1);"

		"	vec4 p = vec4(a_pos[0], f, a_pos[1], 1);"
		"   vec3 g = vec3(g_x, 1.0, g_z);"
		"   gl_Position = u_mvp * p;"
		"	v_p = (u_mv*p).xzy;"
		"	v_n = transpose(inverse(mat3(u_mv)))*normalize(g);"
		"	v_n = normalize(v_n);"
		"}"
		;

	const GLchar fsh[] =
		"#version 330\n"
		""
		"in vec3 v_color;"
		"in vec3 v_p;"
		"in vec3 v_n;"
		""
		"layout(location = 0) out vec4 o_color;"
		""
		"void main()"
		"{"
		"   vec3 l = normalize(v_p - vec3(0,1,0));"
		"   vec3 n = normalize(v_n);"
		"   float d = dot(-l, n);"
		"   float diffuze = max(d, 0.1);"
		"   vec3 r = reflect(l,n);"
		"   vec3 e = normalize(-v_p);"
		"   float b = max(dot(r,e),0);"
		"   float s = pow(b, 3.0);"
		"	if (d>0) " //убрать блики с обратной стороны
		"   o_color = vec4(vec3(0,1,0)*diffuze + vec3(1)*s,1.0);"
		"	else "
		"	o_color = vec4(vec3(0,1,0)*diffuze,1.0);"
		"}"
		;

	GLuint vertexShader, fragmentShader;

	vertexShader = createShader(vsh, GL_VERTEX_SHADER);
	fragmentShader = createShader(fsh, GL_FRAGMENT_SHADER);

	g_shaderProgram = createProgram(vertexShader, fragmentShader);
	g_uMV = glGetUniformLocation(g_shaderProgram, "u_mv");
	g_uMVP = glGetUniformLocation(g_shaderProgram, "u_mvp");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	return g_shaderProgram != 0;
}

GLfloat* createVert() {
	int r = 0, x, y;
	GLfloat s = -1.0f, t = 2.0f / sizeV;
	GLfloat *coord = new GLfloat[sizeV + 1];
	for (int i = 0; i <= sizeV; i++) {
		coord[i] = s;
		s += t;
	}
	GLfloat *v = new GLfloat[sizeV*sizeV * 16];
	for (int i = 0; i < sizeV; i++) {
		for (int j = 0; j < sizeV; j++) {
			x = j; y = i;
			for (int k = 0; k < 4; k++) {
				v[r++] = coord[x];
				v[r++] = coord[y];
				v[r++] = coord[x];
				v[r++] = coord[y];
				if (k == 0) x++;
				else if (k == 1) y++;
				else if (k == 2) x--;
			}
		}
	}
	delete[] coord;
	return v;
}

GLuint* createInd() {
	GLuint *ind = new GLuint[sizeV*sizeV * 6];
	int a = 0, b = 1, c = 2, d = 3, r = 0;
	for (int i = 0; i < sizeV; i++)
		for (int j = 0; j < sizeV; j++) {
			ind[r] = a; r++;
			ind[r] = b; r++; b += 4;
			ind[r] = c; r++;
			ind[r] = c; r++; c += 4;
			ind[r] = d; r++; d += 4;
			ind[r] = a; r++; a += 4;
		}
	return ind;
}

bool createModel()
{
	GLfloat *vertices = createVert();

	GLuint *indices = createInd();


	glGenVertexArrays(1, &g_model.vao);
	glBindVertexArray(g_model.vao);

	glGenBuffers(1, &g_model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeV*sizeV * 16 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_model.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeV*sizeV * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
	g_model.indexCount = sizeV * sizeV * 6;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (const GLvoid *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (const GLvoid *)(2 * sizeof(GLfloat)));

	delete[] vertices;
	delete[] indices;

	return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}

bool init()
{
	// Set initial color of color buffer to white.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//тест глубины
	glEnable(GL_DEPTH_TEST);


	return createShaderProgram() && createModel();
}

void reshape(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void draw(float x, float y, float zoom)
{
	// Clear color buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(g_shaderProgram);
	glBindVertexArray(g_model.vao);

	mat4 Projection = perspective(radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	mat4 Model = mat4(1.0f);
	mat4 View = lookAt(
		vec3(3, 3, 3), // Камера находится в мировых координатах (4,3,3)
		vec3(0, 0, 0), // И направлена в начало координат
		vec3(0, 1, 0)  // "Голова" находится сверху
	);


	Model = scale(Model, vec3(zoom));
	Model = rotate(Model, radians(y), vec3(0, 1, 0));
	Model = rotate(Model, radians(x), vec3(1, 0, 0));
	//Model = translate(Model, vec3(-0.5,0,-0.5));

	auto MV = View * Model;

	mat4 mvp = Projection * MV;

	//суем в видеопамять, g_uMVP-идентификатор юниформа, ее означиваем, колво матриц, не транспон...
	//уходит в видеопамять и становится доступоной в шейдере
	glUniformMatrix4fv(g_uMVP, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(g_uMV, 1, GL_FALSE, value_ptr(MV));
	glDrawElements(GL_TRIANGLES, g_model.indexCount, GL_UNSIGNED_INT, NULL);
}

void cleanup()
{
	if (g_shaderProgram != 0)
		glDeleteProgram(g_shaderProgram);
	if (g_model.vbo != 0)
		glDeleteBuffers(1, &g_model.vbo);
	if (g_model.ibo != 0)
		glDeleteBuffers(1, &g_model.ibo);
	if (g_model.vao != 0)
		glDeleteVertexArrays(1, &g_model.vao);
}

bool initOpenGL()
{
	// Initialize GLFW functions.
	if (!glfwInit())
	{
		cout << "Failed to initialize GLFW" << endl;
		return false;
	}

	// Request OpenGL 3.3 without obsoleted functions.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Создаём окно.
	g_window = glfwCreateWindow(800, 600, "OpenGL Test", NULL, NULL);
	if (g_window == NULL)
	{
		cout << "Failed to open GLFW window" << endl;
		glfwTerminate();
		return false;
	}

	// Initialize OpenGL context with.
	glfwMakeContextCurrent(g_window);

	// Set internal GLEW variable to activate OpenGL core profile.
	glewExperimental = true;

	// Initialize GLEW functions.
	if (glewInit() != GLEW_OK)
	{
		cout << "Failed to initialize GLEW" << endl;
		return false;
	}

	// Ensure we can capture the escape key being pressed.
	glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_TRUE);

	// Set callback for framebuffer resizing event.
	glfwSetFramebufferSizeCallback(g_window, reshape);

	return true;
}

void tearDownOpenGL()
{
	// Terminate GLFW.
	glfwTerminate();
}

int main()
{
	// Initialize OpenGL
	if (!initOpenGL())
		return -1;

	// Initialize graphical resources.
	bool isOk = init();

	if (isOk)
	{
		// Main loop until window closed or escape pressed.
		int x = 1, y = 1;
		float zoom = 1.5;
		while (glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(g_window) == 0)
		{
			y++;

			// Draw scene.
			draw(x, y, zoom);

			// Swap buffers.
			glfwSwapBuffers(g_window);
			// Poll window events.
			glfwPollEvents();
			//Sleep(20);
		}
	}

	// Cleanup graphical resources.
	cleanup();

	// Tear down OpenGL.
	tearDownOpenGL();

	return isOk ? 0 : -1;
}
