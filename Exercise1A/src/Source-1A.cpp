//********************************
//Αυτό το αρχείο θα το χρησιμοποιήσετε
// για να υλοποιήσετε την άσκηση 1Α της OpenGL
//
//ΑΜ:5233                         Όνομα:Ελευθεριος Ιωσηφιδης
//ΑΜ:5386                         Όνομα:Δαναη Χαλανδριδου

//*********************************

#include <stdio.h>
#include <stdlib.h>
#include <ctime>/////
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <windows.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;

//******************
// Η LoadShaders είναι black box για σας - αγνοήστε την συνάρτηση Loadshaders

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	//////////////////////////////////////unicode for Έργασία 1Α – ΧΧΧΧ -YYYΥ
	window = glfwCreateWindow(900, 900, u8"\u0388\u03C1\u03B3\u03B1\u03C3\u03AF\u03B1 1\u0391 \u2013 5233 \u2013 5386", NULL, NULL);


	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	/////////////// grey background
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);


	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//***********************************************
	// Οι shaders σας είναι οι 
    // ProjectVertexShader.vertexshader
    // ProjectFragmentShader.fragmentshader

	GLuint programID = LoadShaders("ProjectVertexShader.vertexshader", "ProjectFragmentShader.fragmentshader");
	
    ///////////////////////////////////////////////////////////////////////////////////////	
	/**Το παρακάτω το αγνοείτε - είναι τοποθέτηση κάμερας ***/
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 4.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 0, 30), // Camera  in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // 
	);

	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model; 
	////////////////////////////////////////////////////////////////////////////////////////
	//shape A 
	GLfloat shape[] = {
		//first triangle
		-1.5f, -10.0f, 0.0f,
		 1.5f, -10.0f, 0.0f,
		 1.5f, -8.5f, 0.0f,

		 //second triangle
		-1.5f, -10.0f, 0.0f,
		 1.5f, -8.5f, 0.0f,
		-1.5f, -8.5f, 0.0f,

		//third triangle
		-1.5f, -8.5f, 0.0f,
		 1.5f, -8.5f, 0.0f,
		 0.0f, -7.75f, 0.0f

	};

	GLuint vbuffer;
	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shape), shape, GL_DYNAMIC_DRAW);
	/*******************************************************************/
	// ===== STAR  =====
	GLuint vboStar;
	glGenBuffers(1, &vboStar);

	// 6 triangles * 3 vertices * (x,y,z)
	float star[18 * 3];

	// inline "builder" for a star centered at (X,Y)
	auto makeStar = [&](float X, float Y) {
				////βαση      υψος       
		const float m = 0.5f, n = 1.5f, h = m * 0.5f;
		int k = 0;
		auto V = [&](float x, float y) { star[k++] = x; star[k++] = y; star[k++] = 0.0f; };

		// center square (2 tris)
		V(X - h, Y - h); V(X + h, Y - h); V(X + h, Y + h);
		V(X - h, Y - h); V(X + h, Y + h); V(X - h, Y + h);
		// spikes: up, down, right, left
		V(X - h, Y + h); V(X + h, Y + h); V(X, Y + h + n);
		V(X - h, Y - h); V(X + h, Y - h); V(X, Y - h - n);
		V(X + h, Y - h); V(X + h, Y + h); V(X + h + n, Y);
		V(X - h, Y - h); V(X - h, Y + h); V(X - h - n, Y);
		};
	//this is the randomness
	std::srand((unsigned)std::time(nullptr));
	auto rnd = [](float lo, float hi) { return lo + (hi - lo) * (std::rand() / (float)RAND_MAX); };

	float sx = rnd(-11.0f, 11.0f);
	float sy = rnd(5.0f, 11.0f);
	makeStar(sx, sy);
	//Send that vertex to OpenGl
	glBindBuffer(GL_ARRAY_BUFFER, vboStar);
	glBufferData(GL_ARRAY_BUFFER, sizeof(star), star, GL_DYNAMIC_DRAW);

	// simple timer: new star every ~1s
	double star_t0 = glfwGetTime();
	/*******************************************************************/
	////////////////////////////////
	float moveX = 0.0f;       // current horizontal offset
	const float step = 1.5f;        // movement step = a/2
	const float limit = 11.0f - 1.5f; // window limit minus half the A width

	// Edge-trigger flags (one step per press)
	static bool prevL = false, prevJ = false;
	/////////////////////////////////

	do {
		// ===== STAR  =====
		// Every second, pick a new random position for the star, 
		// rebuild its geometry there, 
		// and send it to the GPU so it shows up in that new spot.”
		// respawn star every ~1 second
		double now = glfwGetTime();
		if (now - star_t0 > 1.0) {
			star_t0 = now;
			sx = rnd(-11.0f, 11.0f);
			sy = rnd(-5.0f, 11.0f);
			makeStar(sx, sy);
			glBindBuffer(GL_ARRAY_BUFFER, vboStar);
			glBufferData(GL_ARRAY_BUFFER, sizeof(star), star, GL_DYNAMIC_DRAW);
		}
		//////////////////////////////////////////////////////////////////
		// --- Κίνηση: 1 βήμα ανά πάτημα ---
		bool L = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
		bool J = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;

		bool moved = false;
		if (L && !prevL) { moveX += step; moved = true; }
		if (J && !prevJ) { moveX -= step; moved = true; }

		// clamp για να μη βγει εκτός
		if (moved) {
			if (moveX > limit) moveX = limit;
			if (moveX < -limit) moveX = -limit;

			// γράψε τις μετατοπισμένες κορυφές στο VBO
			GLfloat movedShape[sizeof(shape) / sizeof(GLfloat)];
			for (int i = 0; i < (int)(sizeof(shape) / sizeof(GLfloat)); i += 3) {
				movedShape[i] = shape[i] + moveX; // x + offset
				movedShape[i + 1] = shape[i + 1];       // y
				movedShape[i + 2] = shape[i + 2];       // z
			}
			glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(movedShape), movedShape);
		}
		prevL = L; prevJ = J;
		///////////////////////////////////////////////////////////////////////////////////
		
		// Clear the screen 
		glClear(GL_COLOR_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);  /// Αυτό αφορά την κάμερα  - το αγνοείτε


		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
		
		glVertexAttribPointer(
			0,                  // attribute 0, must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3*3); // 3 vertices for each triangle -> 9 vertices total for 3 triangles
		glDisableVertexAttribArray(0);
		// draw STAR (18 vertices)
		glBindBuffer(GL_ARRAY_BUFFER, vboStar);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDrawArrays(GL_TRIANGLES, 0, 18);
		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		// 🔹 Check if the "1" key is pressed to close the app
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	} 
	while (glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS &&  glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);
	glDeleteBuffers(1, &vboStar);///STAR

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

