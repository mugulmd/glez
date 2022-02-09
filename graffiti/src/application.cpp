#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <string>

#include "glez.h"
#include "graffiti_scene.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

/* Core program */

enum class Activity 
{
    Select, Paint, Extrude, Cut
};

bool initGLFW();
bool initWindow();
bool initGLEW();
bool initGL();
bool initImGui();
void scroll_callback(GLFWwindow* _window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* _window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* _window, double xpos, double ypos);
void key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods);

static float screenWidth = 1200.f;
static float screenHeight = 900.f;
static GLFWwindow* window;
static graffiti_scene* scene;

static Activity activity = Activity::Select;
std::string activity_name = "Select";
static glm::vec2 ref_pos;
static bool is_dragging = false;
static bool is_rotating = false;

int main(int argc, char** argv)
{
    if (!initGLFW()) return -1;
    if (!initWindow()) return -1;
    if (!initGLEW()) return -1;
    if (!initGL()) return -1;
    if (!initImGui()) return -1;

    glez::init();
    scene = new graffiti_scene();
    scene->get_camera()->set_proj_props(screenWidth, screenHeight);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        scene->display();

        ImGui::Begin("Tools");
        ImGui::Text(activity_name.c_str());
        ImGui::ColorEdit4("Paint Color", op_paint::color);
        ImGui::SliderFloat("Spray Radius", &op_spray::radius, 0.001f, 0.1f);
        ImGui::SliderInt("N Rays", &op_spray::n_ray, 1, 50);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    delete scene;
	return 0;
}

/* Basic initializations */

bool initGLFW()
{
    if (!glfwInit())
        return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    return true;
}

bool initWindow()
{
    window = glfwCreateWindow((int)screenWidth, (int)screenHeight, "graffiti", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);
    return true;
}

bool initGLEW()
{
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        return false;
    }
    return true;
}

bool initGL()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    return true;
}

bool initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450 core");
    return true;
}

/* Callbacks */

void scroll_callback(GLFWwindow* _window, double xoffset, double yoffset)
{
    float lambda = 1.f - (float)yoffset * 0.1f;
    scene->get_camera()->scale_view(lambda);
}

void mouse_button_callback(GLFWwindow* _window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            is_dragging = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            glm::vec2 cur_pos = glm::vec2(2 * ((float)xpos / screenWidth) - 1,
                1 - 2 * ((float)ypos / screenHeight));
            ref_pos = cur_pos;
            if (mods == GLFW_MOD_ALT) {
                is_rotating = true;
            }
            else {
                switch (activity) {
                case Activity::Select:
                    if (mods == GLFW_MOD_SHIFT) scene->add_to_selection(cur_pos);
                    else scene->set_selection(ref_pos);
                    break;
                case Activity::Paint:
                    scene->paint(cur_pos);
                    break;
                case Activity::Extrude:
                    scene->init_extrude(cur_pos);
                    break;
                case Activity::Cut:
                    scene->cut(cur_pos);
                    break;
                default:
                    break;
                }
            }
        }
        else {
            is_dragging = false;
            is_rotating = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* _window, double xpos, double ypos)
{
    if (is_dragging) {
        glm::vec2 cur_pos = glm::vec2(2 * ((float)xpos / screenWidth) - 1,
            1 - 2 * ((float)ypos / screenHeight));
        glm::vec2 delta = cur_pos - ref_pos;
        if (is_rotating) {
            GLfloat theta_x = delta.x * 3.14f * 0.5f;
            GLfloat theta_y = delta.y * 3.14f * 0.5f;
            scene->get_camera()->rotate_view(-theta_x, -theta_y);
        }
        else {
            switch (activity) {
            case Activity::Paint:
                scene->paint(cur_pos);
                break;
            case Activity::Extrude:
                scene->extrude(ref_pos, cur_pos);
                break;
            default:
                break;
            }
        }
        ref_pos = cur_pos;
    }
}

void key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_H && action == GLFW_RELEASE) {
        // help
        std::cout << "H: help" << std::endl;
        std::cout << "I: info" << std::endl;
        std::cout << "Alt-click-drag: rotate camera" << std::endl;
        std::cout << "Scroll: zoom in/out" << std::endl;
        std::cout << "S: select mode" << std::endl;
        std::cout << "F: fill mode" << std::endl;
        std::cout << "P: spraypaint mode" << std::endl;
        std::cout << "E: extrude mode" << std::endl;
        std::cout << "C: cut mode" << std::endl;
    }
    else if (key == GLFW_KEY_I && action == GLFW_RELEASE) {
        if (activity == Activity::Select && mods == GLFW_MOD_CONTROL) {
            // invert selection
            scene->invert_selection();
        }
        else {
            // object info
            scene->get_camera()->log_info();
            scene->log_object_info();
        }
    }
    else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        if (activity == Activity::Select && mods == GLFW_MOD_CONTROL) {
            // select all
            scene->select_all();
        }
    }
    else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        // select mode
        activity = Activity::Select;
        activity_name = "Select";
        scene->show_selection();
    }
    else if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
        // fill mode
        activity = Activity::Paint;
        activity_name = "Fill";
        scene->set_painter_fill();
    }
    else if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
        // spraypaint mode
        activity = Activity::Paint;
        activity_name = "Spray Paint";
        scene->set_painter_spray();
    }
    else if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
        // extrude mode
        activity = Activity::Extrude;
        activity_name = "Extrude";
        scene->set_extrude();
    }
    else if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
        // cut mode
        activity = Activity::Cut;
        activity_name = "Cut";
        scene->set_cut();
    }
}
