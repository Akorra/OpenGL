#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define IMGUI_IMPL_OPENGL_LOADER_GLEW

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGl", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    /* glew needs a valid ogl rendering context */
    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    { //Scope to force delete of stack allocated buffers
    //Triangle x1, y1, x2, y2, x3, y3
        float positions[] = {
             100.0f, 100.0f, 0.0f, 0.0f, //0
             200.0f, 100.0f, 1.0f, 0.0f, //1
             200.0f, 200.0f, 1.0f, 1.0f, //2
             100.0f, 200.0f, 0.0f, 1.0f  //3
        };

        unsigned int indices[] = {
            0, 1, 2, //triangle 1
            2, 3, 0  //triangle 2
        };

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        VertexBuffer vb(positions, 4 * 4 * sizeof(float)); // 4 points of 4 coords (2 position + 2 texture uv)

        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(2);
        
        VertexArray va;
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        glm::mat4 proj  = glm::ortho(0.0f, 960.0f, 0.5f, 540.0f, -1.0f, 1.0f); 
        glm::mat4 view  = glm::translate(glm::mat4(1.0f), glm::vec3(-100, 0, 0));

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();
        shader.SetUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);

        Texture texture("res/textures/dickbutt.png");
        texture.Bind();

        //Tell our shader wich texture slot to sample from (same slot as passed to texture Bind)
        shader.SetUniform1i("u_Texture", 0);

        //Unbind everything
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        Renderer renderer;
        renderer.SetClearColor(0.13f, 0.13f, 0.13f, 1.0f);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Our ImGui state
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        float r = 0.0f;
        float increment = 0.05f;

        glm::vec3 translation(200, 200, 0);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);

            glm::mat4 mvp = proj * view * model; // opengl matrix multiplication is right to left

            shader.Bind();
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);
            shader.SetUniformMat4f("u_MVP", mvp);

            renderer.Draw(va, ib, shader);

            if (r > 1.0f)
                increment = -0.05f;
            else if (r < 0.0f)
                increment = 0.05f;

            r += increment;

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
            {
                ImGui::Begin("Move Object");                          // Create a window called "Hello, world!" and append into it.

                ImGui::Text("Set Object world position.");               // Display some text (you can use a format strings too)

                ImGui::SliderFloat("X", &translation.x, 0.0f, 960.0f);
                ImGui::SliderFloat("Y", &translation.y, 0.5f, 540.0f);
                ImGui::SliderFloat("Z", &translation.z, -1.0f, 1.0f);
                //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a colo

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}