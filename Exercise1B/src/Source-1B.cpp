//********************************
// Αυτό το αρχείο θα το χρησιμοποιήσετε
// για να υλοποιήσετε την άσκηση 1B της OpenGL
//
// ΑΜ:5233                         Όνομα:Ελευθέριος Ιωσηφίδης
// ΑΜ:5386                         Όνομα:Δανάη Χανλαρίδου
//*********************************

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath> // for std::fmod

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


#include <glm/gtc/constants.hpp> // για glm::two_pi<float>()
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


/// Matrices controlled ONLY by camera_function()
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() { return ViewMatrix; }
glm::mat4 getProjectionMatrix() { return ProjectionMatrix; }

////////////////////////////////////////////// <summary>
/// Add camera function here
/// </summary>

void camera_function()
{
    // FOV σταθερό 60°, aspect=1 (850x850)
    static bool init = false;
    static const float FOVdeg = 60.0f;
    static const float aspect = 1.0f; // 850/850
    if (!init) {
        ProjectionMatrix = glm::perspective(glm::radians(FOVdeg), aspect, 0.1f, 200.0f);
        init = true;
    }

    // --- Κατάσταση κάμερας (orbit) ---
    static float r = glm::length(glm::vec3(0.0f, -5.0f, 20.0f));
    static float pitch = std::asin(-5.0f / r);
    static float yaw   = 0.0f;

    // --- Χρονισμός για ομαλή κίνηση ---
    static double tPrev = glfwGetTime();
    double tNow = glfwGetTime();
    float dt = float(tNow - tPrev);
    tPrev = tNow;

    // --- Ταχύτητες ---
    const float yawSpeed   = glm::radians(90.0f);   // deg/s γύρω από y (Q/Z)
    const float pitchSpeed = glm::radians(90.0f);   // deg/s γύρω από x (W/X)
    const float zoomSpeed  = 10.0f;                 // units/s

    // --- Συνεχής έλεγχος πλήκτρων ---
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) yaw   += yawSpeed   * dt;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) yaw   -= yawSpeed   * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pitch += pitchSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) pitch -= pitchSpeed * dt;

    if (glfwGetKey(window, GLFW_KEY_KP_ADD)      == GLFW_PRESS) r -= zoomSpeed * dt; // zoom in
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) r += zoomSpeed * dt; // zoom out
    r = glm::clamp(r, 2.0f, 80.0f);

    // --- Wrap και στους δύο άξονες (πλήρης 360° επαναλαμβανόμενα) ---
    const float TWO_PI = glm::two_pi<float>();
    yaw   = std::fmod(yaw,   TWO_PI); if (yaw   < 0.0f) yaw   += TWO_PI;
    pitch = std::fmod(pitch, TWO_PI); if (pitch < 0.0f) pitch += TWO_PI;

    // --- Προσανατολισμός με quaternions (χωρίς gimbal lock) ---
    // σειρά: yaw γύρω από world-Y και μετά pitch γύρω από local-X
    glm::quat qYaw   = glm::angleAxis(yaw,   glm::vec3(0,1,0));
    glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1,0,0));
    glm::quat q = qYaw * qPitch;

    // Από τον προσανατολισμό βγάζουμε eye & up
    glm::vec3 eye = q * glm::vec3(0, 0, r);     // περιστρεφουμε το (0,0,r)
    glm::vec3 up  = q * glm::vec3(0, 1, 0);     // το “πάνω” της κάμερας

    ViewMatrix = glm::lookAt(eye, glm::vec3(0,0,0), glm::normalize(up));
}


GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr; sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else {
        printf("Impossible to open %s.\n", vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr; sstr << FragmentShaderStream.rdbuf();
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

///////////////////////////////////////////////////
// --- A dimensions (given) ---
static const float A_len = 3.0f;          // a
static const float A_hgt = A_len * 0.5f;  // b = a/2 = 1.5
static const float A_dep = 2.0f;          // c = 2
static const float A_roof = A_len * 0.25f; // h = a/4 = 0.75

static void tri(vector<glm::vec3>& v, glm::vec3 A, glm::vec3 B, glm::vec3 C) {
    v.push_back(A); v.push_back(B); v.push_back(C);
}
static void tint(vector<glm::vec4>& c, glm::vec4 col) {
    c.push_back(col); c.push_back(col); c.push_back(col);
}

// Build the character A vertices (base box + 4 roof tris) from the spec
static void build_A(vector<glm::vec3>& verts, vector<glm::vec4>& cols) {
    // apex v9 (your spec with back at z = -2 and apex z = -1)
    glm::vec3 v9(0.0f, -7.75f, -1.0f);

    float yTop = v9.y - A_roof; // -8.50
    float yBot = yTop - A_hgt;  // -10.00
    float xL = -A_len * 0.5f;   // -1.5
    float xR = A_len * 0.5f;   //  1.5
    float zF = 0.0f;           // front
    float zB = -A_dep;          // back = -2

    glm::vec3 v1(xL, yBot, zF), v2(xR, yBot, zF), v3(xR, yTop, zF), v4(xL, yTop, zF);
    glm::vec3 v5(xL, yBot, zB), v6(xR, yBot, zB), v7(xR, yTop, zB), v8(xL, yTop, zB);

    // Base faces (each side a different color)
    glm::vec4 cFront(1.0, 0.0, 0.0, 1.0);   // bright red
    glm::vec4 cBack(0.0, 0.8, 0.0, 1.0);    // green
    glm::vec4 cLeft(0.0, 0.4, 1.0, 1.0);    // blue-cyan
    glm::vec4 cRight(1.0, 0.6, 0.0, 1.0);   // orange
    glm::vec4 cTop(0.8, 0.0, 0.8, 1.0);     // violet
    glm::vec4 cBot(0.4, 0.4, 0.4, 1.0);     // gray
    // Roof triangles (four distinct colors)
    glm::vec4 r1(0.0, 1.0, 1.0, 1.0);       // cyan
    glm::vec4 r2(1.0, 0.0, 1.0, 1.0);       // magenta
    glm::vec4 r3(1.0, 1.0, 0.0, 1.0);       // yellow
    glm::vec4 r4(0.6, 0.2, 1.0, 1.0);       // purple-blue


    // Base (12 tris)
    // front z=0
    tri(verts, v1, v2, v3); tint(cols, cFront);
    tri(verts, v1, v3, v4); tint(cols, cFront);
    // back z=-2
    tri(verts, v5, v6, v7); tint(cols, cBack);
    tri(verts, v5, v7, v8); tint(cols, cBack);
    // left x=xL
    tri(verts, v1, v4, v8); tint(cols, cLeft);
    tri(verts, v1, v8, v5); tint(cols, cLeft);
    // right x=xR
    tri(verts, v2, v6, v7); tint(cols, cRight);
    tri(verts, v2, v7, v3); tint(cols, cRight);
    // top y=yTop
    tri(verts, v4, v3, v7); tint(cols, cTop);
    tri(verts, v4, v7, v8); tint(cols, cTop);
    // bottom y=yBot
    tri(verts, v1, v5, v6); tint(cols, cBot);
    tri(verts, v1, v6, v2); tint(cols, cBot);

    // Roof (4 tris) with apex v9
    tri(verts, v4, v3, v9); tint(cols, r1);
    tri(verts, v3, v7, v9); tint(cols, r2);
    tri(verts, v7, v8, v9); tint(cols, r3);
    tri(verts, v8, v4, v9); tint(cols, r4);
}

// one cube of size m with top-front-left corner K
static void build_cube(vector<glm::vec3>& out, glm::vec3 K, float m) {
    glm::vec3 A = K, B = K + glm::vec3(m, 0, 0), C = K + glm::vec3(m, -m, 0), D = K + glm::vec3(0, -m, 0);
    glm::vec3 E = K + glm::vec3(0, 0, m), F = K + glm::vec3(m, 0, m), G = K + glm::vec3(m, -m, m), H = K + glm::vec3(0, -m, m);
    auto T = [&](glm::vec3 p, glm::vec3 q, glm::vec3 r) { tri(out, p, q, r); };
    T(A, B, C); T(A, C, D);         // front
    T(E, G, H); T(E, F, G);         // back
    T(A, D, H); T(A, H, E);         // left
    T(B, F, G); T(B, G, C);         // right
    T(A, E, F); T(A, F, B);         // top
    T(D, C, G); T(D, G, H);         // bottom
}

int main(void)
{
    //(i) Window 850x850, dark gray bg, Greek title
    if (!glfwInit()) { fprintf(stderr, "Failed to initialize GLFW\n"); return -1; }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(850, 850,
        u8"\u0395\u03C1\u03B3\u03B1\u03C3\u03AF\u03B1 1\u0392 \u2013 2025 \u2013 \u03A3\u03BA\u03B7\u03BD\u03B9\u03BA\u03CC \u03A0\u03B1\u03B9\u03C7\u03BD\u03B9\u03B4\u03B9\u03BF\u03CD",
        NULL, NULL);

    if (!window) { fprintf(stderr, "Failed to open GLFW window\n"); glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) { fprintf(stderr, "Failed to initialize GLEW\n"); glfwTerminate(); return -1; }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // dark grey
    glEnable(GL_DEPTH_TEST);

    GLuint vao; glGenVertexArrays(1, &vao); glBindVertexArray(vao);

    GLuint programID = LoadShaders("P1BVertexShader.vertexshader", "P1BFragmentShader.fragmentshader");
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    //(ii)+(iii) Character A geometry + per-face colors
    vector<glm::vec3> Averts; vector<glm::vec4> Acols;
    build_A(Averts, Acols);

    GLuint vboA, cboA;
    glGenBuffers(1, &vboA); glBindBuffer(GL_ARRAY_BUFFER, vboA);
    glBufferData(GL_ARRAY_BUFFER, Averts.size() * sizeof(glm::vec3), Averts.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &cboA); glBindBuffer(GL_ARRAY_BUFFER, cboA);
    glBufferData(GL_ARRAY_BUFFER, Acols.size() * sizeof(glm::vec4), Acols.data(), GL_STATIC_DRAW);

    //(vi) 5 cubes above A
    vector<glm::vec3> cubes;
    const float m = 2.0f, n = 2.0f, stride = m + n; // 4.0
    glm::vec3 K0(-9.0f, 10.0f, 0.0f);               // first cube K
    for (int i = 0; i < 5; i++) build_cube(cubes, K0 + glm::vec3(stride * i, 0, 0), m);

    GLuint vboCubes; glGenBuffers(1, &vboCubes); glBindBuffer(GL_ARRAY_BUFFER, vboCubes);
    glBufferData(GL_ARRAY_BUFFER, cubes.size() * sizeof(glm::vec3), cubes.data(), GL_STATIC_DRAW);

    // No other camera state in main — camera_function() is the only camera.

    // (v) A moves by a/2 on X with J/L (edge-trigger)
    float AoffsetX = 0.0f; const float step = A_len * 0.5f; // 1.5
    bool pL = false, pJ = false;

    while (!glfwWindowShouldClose(window))
    {
        // quit with '1'
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);

        // move J/L one step per press
        bool L = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
        bool J = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
        if (L && !pL) AoffsetX += step;
        if (J && !pJ) AoffsetX -= step;
        pL = L; pJ = J;

        // Update camera once per frame
        camera_function();
        glm::mat4 P = getProjectionMatrix();
        glm::mat4 V = getViewMatrix();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);

        // draw A
        glm::mat4 ModelA = glm::translate(glm::mat4(1.0f), glm::vec3(AoffsetX, 0, 0));
        glm::mat4 MVP_A = P * V * ModelA;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_A[0][0]);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vboA);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, cboA);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)Averts.size());
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // draw cubes (single solid color via constant attrib)
        glm::mat4 MVP_I = P * V * glm::mat4(1.0f);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_I[0][0]);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vboCubes);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDisableVertexAttribArray(1);
        glVertexAttrib4f(1, 1.0f, 1.0f, 0.0f, 1.0f); // yellow cubes

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)cubes.size());
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vboA);
    glDeleteBuffers(1, &cboA);
    glDeleteBuffers(1, &vboCubes);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);
    glfwTerminate();
    return 0;
}
