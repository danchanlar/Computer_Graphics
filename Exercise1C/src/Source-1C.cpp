//********************************
// Εργασία 1Γ – OpenGL
// ΑΜ:5233   Όνομα: Ελευθέριος Ιωσηφίδης
// ΑΜ:5386   Όνομα: Δανάη Χανλαρίδου
//********************************

// ========== Standard headers ==========
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath> 

#ifdef _WIN32
#include <windows.h>   // για Beep(...)
#endif

// ========== OpenGL / GLFW / GLM ==========
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>   // glm::two_pi
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace glm;
using namespace std;

// ========== STB image loader ==========
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//==================================================
// Global window pointer
//==================================================
GLFWwindow* window = nullptr;

//==================================================
// Global matrices (controlled μόνο από camera_function)
//==================================================
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

//==================================================
// Global camera & light
//==================================================
glm::vec3 gCameraPos(0.0f);               // θέση κάμερας για Phong
glm::vec3 gLightPos(8.0f, 0.0f, 0.0f);    // σημειακή φωτεινή πηγή

//==================================================
// Character A (όπως στην 1Β)
//==================================================
static const float A_len = 3.0f;          // a
static const float A_hgt = A_len * 0.5f;  // b = a/2 = 1.5
static const float A_dep = 2.0f;          // c = 2
static const float A_roof = A_len * 0.25f; // h = a/4 = 0.75

// Για συγκρούσεις με εχθρούς
static const float A_xL = -1.5f;
static const float A_xR = 1.5f;
static const float A_yTop = -8.5f;
static const float A_yBot = -10.0f;

// Το apex (v9) στο local space του Α
static const glm::vec3 A_apex_local(0.0f, -7.75f, -1.0f);

//==================================================
// Bullet struct & helper για AABB
//==================================================
struct Bullet {
    glm::vec3 pos;
    bool      active;
};

// helper για 1D overlap (για AABB σε x,y)
inline bool overlap1D(float aMin, float aMax, float bMin, float bMax) {
    return (aMax >= bMin) && (bMax >= aMin);
}

//==================================================
// Game over flash variable (οπτικό εφέ)
//==================================================
float gameOverFlash = 0.0f; // 0 = τίποτα, 1 = κόκκινο πλήρως

//==================================================
// Enemy types & Mesh struct
//==================================================
enum EnemyShape {
    SHAPE_SPHERE = 0,
    SHAPE_TORUS,
    SHAPE_CYLINDER,
    SHAPE_CONE
};

struct Mesh {
    GLuint  vboPos = 0;
    GLuint  vboNorm = 0;
    GLsizei vertexCount = 0;
};

//==================================================
// Forward declarations (helpers)
//==================================================
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

void camera_function();

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);
GLuint LoadTexture(const char* filename);

void tri(std::vector<glm::vec3>& v, glm::vec3 A, glm::vec3 B, glm::vec3 C);
void tri_uv(std::vector<glm::vec2>& t, glm::vec2 A, glm::vec2 B, glm::vec2 C);

void triPN(std::vector<glm::vec3>& pos,
    std::vector<glm::vec3>& norms,
    const glm::vec3& A,
    const glm::vec3& B,
    const glm::vec3& C);

void build_A(std::vector<glm::vec3>& verts,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& norms);

void build_box_center(std::vector<glm::vec3>& outPos,
    std::vector<glm::vec3>& outNorm,
    glm::vec3 h);

bool LoadOBJ(const char* path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec3>& out_normals,
    std::vector<glm::vec2>& out_uvs);

bool CreateMeshFromOBJ(const char* path, Mesh& mesh);

// μικρές βοηθητικές που πρόσθεσα για ευαναγνωσιμότητα
void updateLightPosition(float dt, glm::vec3& lightPos, float moveSpeed);
void updateGameOverFlash(float dt, float& flashValue);
void playGameOverBeep();

//==================================================
// Getter functions for matrices
//==================================================
glm::mat4 getViewMatrix() {
    return ViewMatrix;
}

glm::mat4 getProjectionMatrix() {
    return ProjectionMatrix;
}

//==================================================
// Camera orbit γύρω από το (0,0,0)
//==================================================
void camera_function() {
    // FOV σταθερό 60°, aspect = 1 (850x850)
    static bool  init = false;
    static const float FOVdeg = 60.0f;
    static const float aspect = 1.0f; // 850 / 850

    if (!init) {
        ProjectionMatrix = glm::perspective(glm::radians(FOVdeg), aspect, 0.1f, 200.0f);
        init = true;
    }

    // κατάσταση κάμερας (orbit)
    static float r = glm::length(glm::vec3(0.0f, -5.0f, 20.0f));
    static float pitch = std::asin(-5.0f / r);
    static float yaw = 0.0f;

    // χρονισμός για ομαλή κίνηση
    static double tPrev = glfwGetTime();
    double tNow = glfwGetTime();
    float dt = static_cast<float>(tNow - tPrev);
    tPrev = tNow;

    // ταχύτητες
    const float yawSpeed = glm::radians(90.0f);  // Q / Z
    const float pitchSpeed = glm::radians(90.0f);  // W / X
    const float zoomSpeed = 10.0f;                // units / sec

    // έλεγχος πλήκτρων
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) yaw += yawSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) yaw -= yawSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pitch += pitchSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) pitch -= pitchSpeed * dt;

    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)      r -= zoomSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) r += zoomSpeed * dt;
    r = glm::clamp(r, 2.0f, 80.0f);

    // wrap [0, 2π)
    const float TWO_PI = glm::two_pi<float>();
    yaw = std::fmod(yaw, TWO_PI);   if (yaw < 0.0f) yaw += TWO_PI;
    pitch = std::fmod(pitch, TWO_PI); if (pitch < 0.0f) pitch += TWO_PI;

    // προσανατολισμός με quaternions
    glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
    glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
    glm::quat q = qYaw * qPitch;

    glm::vec3 eye = q * glm::vec3(0, 0, r);
    glm::vec3 up = q * glm::vec3(0, 1, 0);

    ViewMatrix = glm::lookAt(eye, glm::vec3(0, 0, 0), glm::normalize(up));
    gCameraPos = eye;
}

//==================================================
// Shader loading
//==================================================
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // vertex shader code
    std::string VertexShaderCode;
    {
        std::ifstream stream(vertex_file_path, std::ios::in);
        if (!stream.is_open()) {
            printf("Impossible to open %s.\n", vertex_file_path);
            return 0;
        }
        std::stringstream sstr;
        sstr << stream.rdbuf();
        VertexShaderCode = sstr.str();
    }

    // fragment shader code
    std::string FragmentShaderCode;
    {
        std::ifstream stream(fragment_file_path, std::ios::in);
        if (!stream.is_open()) {
            printf("Impossible to open %s.\n", fragment_file_path);
            return 0;
        }
        std::stringstream sstr;
        sstr << stream.rdbuf();
        FragmentShaderCode = sstr.str();
    }

    GLint Result = GL_FALSE;
    int   InfoLogLength = 0;

    // compile vertex
    printf("Compiling shader : %s\n", vertex_file_path);
    const char* vsrc = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &vsrc, nullptr);
    glCompileShader(VertexShaderID);

    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> msg(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, msg.data());
        printf("%s\n", msg.data());
    }

    // compile fragment
    printf("Compiling shader : %s\n", fragment_file_path);
    const char* fsrc = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &fsrc, nullptr);
    glCompileShader(FragmentShaderID);

    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> msg(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, msg.data());
        printf("%s\n", msg.data());
    }

    // link program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> msg(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, msg.data());
        printf("%s\n", msg.data());
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

//==================================================
// Φόρτωση textureA.jpg
//==================================================
GLuint LoadTexture(const char* filename) {
    int w, h, n;
    stbi_set_flip_vertically_on_load(1);

    unsigned char* data = stbi_load(filename, &w, &h, &n, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    GLenum format = (n == 3) ? GL_RGB : GL_RGBA;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // basic parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}

//==================================================
// Μικρές helpers για τρίγωνα & UVs & normals
//==================================================
void tri(std::vector<glm::vec3>& v,
    glm::vec3 A, glm::vec3 B, glm::vec3 C) {
    v.push_back(A);
    v.push_back(B);
    v.push_back(C);
}

void tri_uv(std::vector<glm::vec2>& t,
    glm::vec2 A, glm::vec2 B, glm::vec2 C) {
    t.push_back(A);
    t.push_back(B);
    t.push_back(C);
}

// τρίγωνο με αυτόματα υπολογισμένο normal
void triPN(std::vector<glm::vec3>& pos,
    std::vector<glm::vec3>& norms,
    const glm::vec3& A,
    const glm::vec3& B,
    const glm::vec3& C) {
    glm::vec3 n = glm::normalize(glm::cross(B - A, C - A));

    pos.push_back(A);
    pos.push_back(B);
    pos.push_back(C);

    norms.push_back(n);
    norms.push_back(n);
    norms.push_back(n);
}

//==================================================
// Δημιουργία γεωμετρίας για το Character A
//==================================================
void build_A(std::vector<glm::vec3>& verts,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& norms) {
    glm::vec3 v9 = A_apex_local;

    float yTop = v9.y - A_roof; // -8.50
    float yBot = yTop - A_hgt;  // -10.00
    float xL = -A_len * 0.5f; // -1.5
    float xR = A_len * 0.5f; //  1.5
    float zF = 0.0f;          // front
    float zB = -A_dep;        // back = -2

    glm::vec3 v1(xL, yBot, zF), v2(xR, yBot, zF), v3(xR, yTop, zF), v4(xL, yTop, zF);
    glm::vec3 v5(xL, yBot, zB), v6(xR, yBot, zB), v7(xR, yTop, zB), v8(xL, yTop, zB);

    // UVs: βασικά τετράπλευρα
    glm::vec2 t1(0, 0), t2(1, 0), t3(1, 1), t4(0, 1);

    // UVs για πλευρές
    glm::vec2 l1(0, 0), l2(0, 1), l3(1, 1), l4(1, 0);

    // UVs για σκεπή
    glm::vec2 rA(0, 0), rB(1, 0), rC(0.5f, 1.0f);

    // front
    triPN(verts, norms, v1, v2, v3); tri_uv(uvs, t1, t2, t3);
    triPN(verts, norms, v1, v3, v4); tri_uv(uvs, t1, t3, t4);

    // back
    triPN(verts, norms, v5, v6, v7); tri_uv(uvs, t1, t2, t3);
    triPN(verts, norms, v5, v7, v8); tri_uv(uvs, t1, t3, t4);

    // left
    triPN(verts, norms, v1, v4, v8); tri_uv(uvs, l1, l2, l3);
    triPN(verts, norms, v1, v8, v5); tri_uv(uvs, l1, l3, l4);

    // right
    triPN(verts, norms, v2, v6, v7); tri_uv(uvs, l1, l4, l3);
    triPN(verts, norms, v2, v7, v3); tri_uv(uvs, l1, l3, l2);

    // top
    triPN(verts, norms, v4, v3, v7); tri_uv(uvs, t1, t2, t3);
    triPN(verts, norms, v4, v7, v8); tri_uv(uvs, t1, t3, t4);

    // bottom
    triPN(verts, norms, v1, v5, v6); tri_uv(uvs, t1, t4, t3);
    triPN(verts, norms, v1, v6, v2); tri_uv(uvs, t1, t3, t2);

    // roof
    triPN(verts, norms, v4, v3, v9); tri_uv(uvs, rA, rB, rC);
    triPN(verts, norms, v3, v7, v9); tri_uv(uvs, rA, rB, rC);
    triPN(verts, norms, v7, v8, v9); tri_uv(uvs, rA, rB, rC);
    triPN(verts, norms, v8, v4, v9); tri_uv(uvs, rA, rB, rC);
}

//==================================================
// Box centered στο (0,0,0) με half-dim h
//==================================================
void build_box_center(std::vector<glm::vec3>& outPos,
    std::vector<glm::vec3>& outNorm,
    glm::vec3 h) {
    glm::vec3 A(-h.x, +h.y, +h.z);
    glm::vec3 B(+h.x, +h.y, +h.z);
    glm::vec3 C(+h.x, -h.y, +h.z);
    glm::vec3 D(-h.x, -h.y, +h.z);
    glm::vec3 E(-h.x, +h.y, -h.z);
    glm::vec3 F(+h.x, +h.y, -h.z);
    glm::vec3 G(+h.x, -h.y, -h.z);
    glm::vec3 H(-h.x, -h.y, -h.z);

    auto T = [&](glm::vec3 p, glm::vec3 q, glm::vec3 r) {
        triPN(outPos, outNorm, p, q, r);
        };

    // front (z = +)
    T(A, B, C); T(A, C, D);
    // back  (z = -)
    T(E, G, H); T(E, F, G);
    // left  (x = -)
    T(A, D, H); T(A, H, E);
    // right (x = +)
    T(B, F, G); T(B, G, C);
    // top   (y = +)
    T(A, E, F); T(A, F, B);
    // bottom(y = -)
    T(D, C, G); T(D, G, H);
}

//==================================================
// OBJ loader (triangles only)
//==================================================
bool LoadOBJ(const char* path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec3>& out_normals,
    std::vector<glm::vec2>& out_uvs) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Cannot open OBJ file: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_pos;
    std::vector<glm::vec3> temp_norm;
    std::vector<glm::vec2> temp_uv;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            temp_pos.push_back(p);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_uv.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            temp_norm.push_back(n);
        }
        else if (prefix == "f") {
            std::vector<std::string> tokens;
            std::string vertStr;
            while (ss >> vertStr)
                tokens.push_back(vertStr);

            if (tokens.size() < 3) continue;

            auto parseVertex = [&](const std::string& vstr,
                int& vi, int& ti, int& ni) {
                    vi = ti = ni = 0;
                    std::string s = vstr;
                    for (char& c : s) {
                        if (c == '/') c = ' ';
                    }
                    std::stringstream vs(s);
                    vs >> vi;
                    if (!(vs >> ti)) ti = 0;
                    if (!(vs >> ni)) ni = 0;
                };

            // fan triangulation
            for (size_t i = 1; i + 1 < tokens.size(); ++i) {
                int vIdx[3], tIdx[3], nIdx[3];

                parseVertex(tokens[0], vIdx[0], tIdx[0], nIdx[0]);
                parseVertex(tokens[i], vIdx[1], tIdx[1], nIdx[1]);
                parseVertex(tokens[i + 1], vIdx[2], tIdx[2], nIdx[2]);

                for (int k = 0; k < 3; ++k) {
                    glm::vec3 pos = temp_pos[vIdx[k] - 1];
                    glm::vec3 nor(0.0f, 1.0f, 0.0f);
                    glm::vec2 uv(0.0f, 0.0f);

                    if (nIdx[k] > 0 && nIdx[k] <= static_cast<int>(temp_norm.size()))
                        nor = temp_norm[nIdx[k] - 1];

                    if (tIdx[k] > 0 && tIdx[k] <= static_cast<int>(temp_uv.size()))
                        uv = temp_uv[tIdx[k] - 1];

                    out_vertices.push_back(pos);
                    out_normals.push_back(nor);
                    out_uvs.push_back(uv);
                }
            }
        }
    }

    std::cout << "Loaded OBJ: " << path
        << " (triangle verts: " << out_vertices.size() << ")\n";
    return true;
}

//==================================================
// Δημιουργία Mesh από OBJ
//==================================================
bool CreateMeshFromOBJ(const char* path, Mesh& mesh) {
    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> norms;
    std::vector<glm::vec2> uvs;

    if (!LoadOBJ(path, verts, norms, uvs)) {
        return false;
    }

    glGenBuffers(1, &mesh.vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboPos);
    glBufferData(GL_ARRAY_BUFFER,
        verts.size() * sizeof(glm::vec3),
        verts.data(),
        GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.vboNorm);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboNorm);
    glBufferData(GL_ARRAY_BUFFER,
        norms.size() * sizeof(glm::vec3),
        norms.data(),
        GL_STATIC_DRAW);

    mesh.vertexCount = static_cast<GLsizei>(verts.size());
    return true;
}

//==================================================
// ΝΕΕΣ μικρές helper συναρτήσεις
//==================================================

// (ε) Κίνηση φωτεινής πηγής με arrow keys + PageUp/PageDown
void updateLightPosition(float dt, glm::vec3& lightPos, float moveSpeed) {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) lightPos.x -= moveSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) lightPos.x += moveSpeed * dt;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) lightPos.z -= moveSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) lightPos.z += moveSpeed * dt;

    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) lightPos.y += moveSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) lightPos.y -= moveSpeed * dt;
}

// Χειρισμός του κόκκινου flash στο game over
void updateGameOverFlash(float dt, float& flashValue) {
    if (flashValue > 0.0f) {
        glClearColor(0.8f * flashValue, 0.0f, 0.0f, 1.0f);
        flashValue -= dt;
        if (flashValue < 0.0f) flashValue = 0.0f;
    }
    else {
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    }
}

// Ήχος στο game over (Windows μόνο)
void playGameOverBeep() {
#ifdef _WIN32
    Beep(750, 200);  // συχνότητα 750 Hz, διάρκεια 200 ms
    Beep(500, 200);  // δεύτερος ήχος
#endif
}

//==================================================
// main
//==================================================
int main() {
    // --- Αρχικοποίηση GLFW / παράθυρο ---
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(
        850, 850,
        u8"Εργασία 1Γ - 2025 - Καταστροφέας",
        nullptr, nullptr);

    if (!window) {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // --- Shaders & uniforms ---
    GLuint programID = LoadShaders("P1CVertexShader.vertexshader",
        "P1CFragmentShader.fragmentshader");

    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ColorID = glGetUniformLocation(programID, "uniColor");
    GLuint UseTexID = glGetUniformLocation(programID, "useTexture");
    GLuint TexID = glGetUniformLocation(programID, "texSampler");
    GLuint ModelID = glGetUniformLocation(programID, "M");
    GLuint NormalMatID = glGetUniformLocation(programID, "NormalMatrix");
    GLuint ViewPosID = glGetUniformLocation(programID, "viewPos");
    GLuint LightPosID = glGetUniformLocation(programID, "lightPos");

    // --- Character A: geometry + UVs + normals ---
    std::vector<glm::vec3> Averts;
    std::vector<glm::vec2> Auvs;
    std::vector<glm::vec3> ANormals;
    build_A(Averts, Auvs, ANormals);

    GLuint vboA_pos, vboA_uv, vboA_norm;
    glGenBuffers(1, &vboA_pos);
    glBindBuffer(GL_ARRAY_BUFFER, vboA_pos);
    glBufferData(GL_ARRAY_BUFFER,
        Averts.size() * sizeof(glm::vec3),
        Averts.data(),
        GL_STATIC_DRAW);

    glGenBuffers(1, &vboA_uv);
    glBindBuffer(GL_ARRAY_BUFFER, vboA_uv);
    glBufferData(GL_ARRAY_BUFFER,
        Auvs.size() * sizeof(glm::vec2),
        Auvs.data(),
        GL_STATIC_DRAW);

    glGenBuffers(1, &vboA_norm);
    glBindBuffer(GL_ARRAY_BUFFER, vboA_norm);
    glBufferData(GL_ARRAY_BUFFER,
        ANormals.size() * sizeof(glm::vec3),
        ANormals.data(),
        GL_STATIC_DRAW);

    GLuint texA = LoadTexture("textureA.jpg");

    // --- Εχθροί: 4 διαφορετικά OBJ models ---
    const int   ENEMY_COUNT = 5;
    const float ENEMY_SIZE = 2.0f;
    const glm::vec3 EnemyHalf(ENEMY_SIZE * 0.5f);

    Mesh meshSphere;
    Mesh meshTorus;
    Mesh meshCylinder;
    Mesh meshCone;

    if (!CreateMeshFromOBJ("enemy_sphere.obj", meshSphere))   return -1;
    if (!CreateMeshFromOBJ("fatTorus.obj", meshTorus))    return -1;
    if (!CreateMeshFromOBJ("enemy_cylinder.obj", meshCylinder)) return -1;
    if (!CreateMeshFromOBJ("enemy_cone.obj", meshCone))     return -1;

    EnemyShape enemyShape[ENEMY_COUNT] = {
        SHAPE_SPHERE,
        SHAPE_TORUS,
        SHAPE_CYLINDER,
        SHAPE_CONE,
        SHAPE_SPHERE
    };

    glm::vec3 enemyPos[ENEMY_COUNT];
    bool      enemyAlive[ENEMY_COUNT];

    const float gap = 2.0f; // όπως στην 1Β

    glm::vec3 firstK(-9.0f, 10.0f, 0.0f);
    glm::vec3 firstCenter(
        firstK.x + ENEMY_SIZE * 0.5f,
        firstK.y - ENEMY_SIZE * 0.5f,
        firstK.z + ENEMY_SIZE * 0.5f
    );

    for (int i = 0; i < ENEMY_COUNT; ++i) {
        enemyPos[i] = firstCenter + glm::vec3(i * (ENEMY_SIZE + gap), 0.0f, 0.0f);
        enemyAlive[i] = true;
    }

    const float ENEMY_STEP = ENEMY_SIZE * 0.5f; // κάθετες κινήσεις
    const float ENEMY_STEP_X = ENEMY_SIZE * 0.5f; // οριζόντιες κινήσεις

    // μοτίβο κίνησης: δεξιά, πίσω, αριστερά, πίσω, κάτω
    int enemyPhase = 0;   // 0..4
    int phaseStepCount = 0;   // βήματα στη φάση
    const int stepsPerSide = 5;

    // --- Bullets: box 0.25 x 0.5 x 0.25 ---
    const glm::vec3 BulletHalf(0.25f * 0.5f,
        0.5f * 0.5f,
        0.25f * 0.5f);

    std::vector<glm::vec3> bulletVerts;
    std::vector<glm::vec3> bulletNorms;
    build_box_center(bulletVerts, bulletNorms, BulletHalf);

    GLuint vboBullet, vboBulletNorm;
    glGenBuffers(1, &vboBullet);
    glBindBuffer(GL_ARRAY_BUFFER, vboBullet);
    glBufferData(GL_ARRAY_BUFFER,
        bulletVerts.size() * sizeof(glm::vec3),
        bulletVerts.data(),
        GL_STATIC_DRAW);

    glGenBuffers(1, &vboBulletNorm);
    glBindBuffer(GL_ARRAY_BUFFER, vboBulletNorm);
    glBufferData(GL_ARRAY_BUFFER,
        bulletNorms.size() * sizeof(glm::vec3),
        bulletNorms.data(),
        GL_STATIC_DRAW);

    std::vector<Bullet> bullets;
    const float BulletSpeed = 8.0f; // units/sec

    // --- Κίνηση χαρακτήρα Α με J/L (ανά A_len/2) ---
    float AoffsetX = 0.0f;
    const float Astep = A_len * 0.5f; // 1.5
    bool pL = false, pJ = false, pSpace = false;

    // --- Κίνηση φωτεινής πηγής ---
    const float lightMoveSpeed = 8.0f;

    bool gameOver = false;

    // χρονισμός για dt & enemy steps
    double lastFrameTime = glfwGetTime();
    double lastEnemyStep = lastFrameTime;

    // --- Pause / Restart / Ταχύτητα εχθρών ---
    bool   paused = false;
    bool   prevP = false, prevR = false;
    bool   prevF = false, prevS = false;
    double enemyStepPeriod = 5.0; // δευτερόλεπτα ανά βήμα

    //==================================================
    // Κύριο game loop
    //==================================================
    while (!glfwWindowShouldClose(window)) {
        // quit με '1'
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        double now = glfwGetTime();
        float dt = static_cast<float>(now - lastFrameTime);
        lastFrameTime = now;

        // (ε) κίνηση φωτεινής πηγής
        updateLightPosition(dt, gLightPos, lightMoveSpeed);

        // --- Ειδικά πλήκτρα: P (pause), R (restart), F/S (ταχύτητα) ---
        bool keyP = (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);
        bool keyR = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
        bool keyF = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
        bool keyS = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);

        // P: toggle pause
        if (keyP && !prevP) {
            paused = !paused;
        }
        prevP = keyP;

        // R: restart παιχνιδιού
        if (keyR && !prevR) {
            gameOver = false;
            paused = false;

            // θέση Α
            AoffsetX = 0.0f;

            // καθάρισμα βλημάτων
            bullets.clear();

            // επαναφορά εχθρών
            for (int i = 0; i < ENEMY_COUNT; ++i) {
                enemyPos[i] = firstCenter + glm::vec3(i * (ENEMY_SIZE + gap), 0.0f, 0.0f);
                enemyAlive[i] = true;
            }

            // reset χρονιστών
            lastEnemyStep = now;
            gameOverFlash = 0.0f;
        }
        prevR = keyR;

        // F: αύξηση ταχύτητας
        if (keyF && !prevF) {
            enemyStepPeriod *= 0.7;
            if (enemyStepPeriod < 0.01) enemyStepPeriod = 0.01;
        }
        prevF = keyF;

        // S: μείωση ταχύτητας
        if (keyS && !prevS) {
            enemyStepPeriod *= 1.3;
            if (enemyStepPeriod > 10.0) enemyStepPeriod = 10.0;
        }
        prevS = keyS;

        // Ενημέρωση κάμερας
        camera_function();
        glm::mat4 P = getProjectionMatrix();
        glm::mat4 V = getViewMatrix();

        // --- Κίνηση χαρακτήρα Α (J/L) ---
        if (!gameOver && !paused) {
            bool L = (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS);
            bool J = (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS);

            if (L && !pL) AoffsetX += Astep;
            if (J && !pJ) AoffsetX -= Astep;

            pL = L;
            pJ = J;
        }

        // --- Κίνηση εχθρών σύμφωνα με το pattern ---
        if (!gameOver && !paused && (now - lastEnemyStep) >= enemyStepPeriod) {
            lastEnemyStep = now;

            for (int i = 0; i < ENEMY_COUNT; ++i) {
                if (!enemyAlive[i]) continue;

                switch (enemyPhase) {
                case 0:  // δεξιά
                    enemyPos[i].x += ENEMY_STEP_X;
                    break;
                case 1:  // πίσω προς αρχική (από δεξιά)
                    enemyPos[i].x -= ENEMY_STEP_X;
                    break;
                case 2:  // αριστερά
                    enemyPos[i].x -= ENEMY_STEP_X;
                    break;
                case 3:  // πίσω προς αρχική (από αριστερά)
                    enemyPos[i].x += ENEMY_STEP_X;
                    break;
                case 4:  // κάτω
                    enemyPos[i].y -= ENEMY_STEP;
                    break;
                }
            }

            phaseStepCount++;

            if (enemyPhase >= 0 && enemyPhase <= 3) {
                if (phaseStepCount >= stepsPerSide) {
                    enemyPhase++;
                    phaseStepCount = 0;
                }
            }
            else if (enemyPhase == 4) {
                enemyPhase = 0;
                phaseStepCount = 0;
            }
        }

        // --- Πυροβολισμός με SPACE ---
        bool Space = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        if (!gameOver && !paused && Space && !pSpace) {
            Bullet b;
            b.pos = glm::vec3(AoffsetX, 0, 0) + A_apex_local;
            b.active = true;
            bullets.push_back(b);
        }
        pSpace = Space;

        // --- Ενημέρωση βλημάτων (+y) ---
        if (!gameOver && !paused) {
            for (auto& b : bullets) {
                if (b.active) {
                    b.pos.y += BulletSpeed * dt;
                }
            }
        }

        // --- Συγκρούσεις βλημάτων - εχθρών ---
        if (!gameOver && !paused) {
            for (auto& b : bullets) {
                if (!b.active) continue;

                float bMinX = b.pos.x - BulletHalf.x;
                float bMaxX = b.pos.x + BulletHalf.x;
                float bMinY = b.pos.y - BulletHalf.y;
                float bMaxY = b.pos.y + BulletHalf.y;

                for (int i = 0; i < ENEMY_COUNT; ++i) {
                    if (!enemyAlive[i]) continue;

                    glm::vec3 c = enemyPos[i];
                    glm::vec3 h = EnemyHalf;

                    float eMinX = c.x - h.x;
                    float eMaxX = c.x + h.x;
                    float eMinY = c.y - h.y;
                    float eMaxY = c.y + h.y;

                    if (overlap1D(bMinX, bMaxX, eMinX, eMaxX) &&
                        overlap1D(bMinY, bMaxY, eMinY, eMaxY)) {
                        enemyAlive[i] = false;
                        b.active = false;
                        break;
                    }
                }
            }
        }

        // --- Έλεγχος αν κάποιος εχθρός φτάνει τον Α ---
        if (!gameOver && !paused) {
            for (int i = 0; i < ENEMY_COUNT; ++i) {
                if (!enemyAlive[i]) continue;

                float enemyBottomY = enemyPos[i].y - EnemyHalf.y;
                if (enemyBottomY <= A_yTop) {
                    gameOver = true;

                    for (auto& b : bullets) {
                        b.active = false;
                    }

                    // ηχητικό εφέ στο game over
                    playGameOverBeep();

                    // κόκκινο flash
                    gameOverFlash = 1.0f;
                    break;
                }
            }
        }

        // --- Background / flash ---
        updateGameOverFlash(dt, gameOverFlash);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);

        // Phong: θέση κάμερας & φωτεινής πηγής
        glUniform3fv(ViewPosID, 1, &gCameraPos.x);
        glUniform3fv(LightPosID, 1, &gLightPos.x);

        // --- Σχεδίαση Character A ---
        if (!gameOver) {
            glm::mat4 ModelA = glm::translate(glm::mat4(1.0f), glm::vec3(AoffsetX, 0, 0));
            glm::mat4 MVP_A = P * V * ModelA;
            glm::mat3 NormalA = glm::mat3(glm::transpose(glm::inverse(ModelA)));

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_A[0][0]);
            glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelA[0][0]);
            glUniformMatrix3fv(NormalMatID, 1, GL_FALSE, &NormalA[0][0]);

            glUniform1i(UseTexID, 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texA);
            glUniform1i(TexID, 0);
            glUniform4f(ColorID, 1.0f, 1.0f, 1.0f, 1.0f);

            // θέση -> location 0
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vboA_pos);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            // normal -> location 1
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, vboA_norm);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            // UV -> location 2
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, vboA_uv);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(Averts.size()));

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);
        }

        // --- Εχθροί (OBJ, κίτρινοι, χωρίς UV) ---
        glUniform1i(UseTexID, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform4f(ColorID, 1.0f, 1.0f, 0.0f, 1.0f);

        glEnableVertexAttribArray(0); // θέση
        glEnableVertexAttribArray(1); // normal
        glDisableVertexAttribArray(2);

        for (int i = 0; i < ENEMY_COUNT; ++i) {
            if (!enemyAlive[i]) continue;

            Mesh* mesh = nullptr;
            switch (enemyShape[i]) {
            case SHAPE_SPHERE:   mesh = &meshSphere;   break;
            case SHAPE_TORUS:    mesh = &meshTorus;    break;
            case SHAPE_CYLINDER: mesh = &meshCylinder; break;
            case SHAPE_CONE:     mesh = &meshCone;     break;
            }
            if (!mesh || mesh->vertexCount == 0) continue;

            glBindBuffer(GL_ARRAY_BUFFER, mesh->vboPos);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glBindBuffer(GL_ARRAY_BUFFER, mesh->vboNorm);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glm::mat4 ModelC = glm::translate(glm::mat4(1.0f), enemyPos[i]);
            glm::mat4 MVP_C = P * V * ModelC;
            glm::mat3 NormalC = glm::mat3(glm::transpose(glm::inverse(ModelC)));

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_C[0][0]);
            glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelC[0][0]);
            glUniformMatrix3fv(NormalMatID, 1, GL_FALSE, &NormalC[0][0]);

            glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // --- Βλήματα (κόκκινα, χωρίς υφή, με Phong) ---
        glUniform1i(UseTexID, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform4f(ColorID, 1.0f, 0.0f, 0.0f, 1.0f);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vboBullet);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, vboBulletNorm);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDisableVertexAttribArray(2);

        for (auto& b : bullets) {
            if (!b.active) continue;

            glm::mat4 ModelB = glm::translate(glm::mat4(1.0f), b.pos);
            glm::mat4 MVP_B = P * V * ModelB;
            glm::mat3 NormalB = glm::mat3(glm::transpose(glm::inverse(ModelB)));

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_B[0][0]);
            glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelB[0][0]);
            glUniformMatrix3fv(NormalMatID, 1, GL_FALSE, &NormalB[0][0]);

            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(bulletVerts.size()));
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // swap buffers + events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ====== ΤΕΛΟΣ GAME LOOP ======

    glDeleteBuffers(1, &vboA_pos);
    glDeleteBuffers(1, &vboA_uv);
    glDeleteBuffers(1, &vboBullet);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);

    glfwTerminate();

    return 0;
}
