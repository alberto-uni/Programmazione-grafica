/*
Nell'esercitazione sono presenti 4 Point Light, Una Directional Light e una luce Spotlight che punta nella stessa direzione della camera
3 delle point light sono statiche mentre una ruota attorno al modello
Le point light sono rappresentate come semplici cubi
Il lighting viene calcolato all'interno del Tangent Space

Il movimento della camera viene gestito dall'utente utlizzando i tasti WASD, mentre la direzione viene decisa dalla posizione del mouse e lo zoom dalla rotellina 
Utilizzo la classe Camera per gestire tutto il movimento della camera
il modello statico utilizzato è il backpack preso dalla dipsensa di LearOpenGL sviluppata da JoeyDeVries https://github.com/JoeyDeVries/LearnOpenGL/tree/master/resources/objects/backpack
e viene caricato tramite l'utlizzo della libreria assimp
*/

//includo le librerie necessarie
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

//imposto dimensione della finestra
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// imposto posizione iniziale della camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;



int main()
{
    // glfw: configurazione
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw: creo la finestra
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // cattura mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //flip sull'asse delle y
    stbi_set_flip_vertically_on_load(true);

    // abilito depth testing. Buffer di profondita'
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // creazione degli shader
    // uno per gestire il lightinge uno per il modello
    // ------------------------------------
    Shader lightingShader("es1Lights.vs", "es1Lights.fs");
    Shader modelShader("es1model_Load.vs", "es1model_Load.fs");
    //percorso modello
    Model ourModel("C:/Users/Baroni Alberto/Desktop/sguola/Magistrale/grafica/backpack/backpack.obj");
    // definizione vertici per il cubo
    // 36 verticci --> 6 vertici per faccia 
    // ------------------------------------------------------------------
    float vertices[] = {
        //Posizioni       
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };
 
    // posizioni Lampadine/Point Lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 2.0,  2.0f,  1.0f),
        glm::vec3( -2.0f, -2.0f, -1.0f),
        glm::vec3(1.0f,  0.0f, 1.0f),
        glm::vec3( 0.0f,  0.0f, 0.0f)
    };
    // configurazione Vertex Array Object(VAO) usato dal cubo

    //Cubo
    unsigned int VBO, modelVAO;
    glGenVertexArrays(1, &modelVAO);
    glGenBuffers(1, &VBO);

    //Faccio i bind per associare la memoria
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //posizione
    glBindVertexArray(modelVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // configurazione Vertex Array Object(VAO) usato dal lighting
    //Light VAO
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    //posizione
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    



   

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // calcolo deltaTime e LastFrame. Utilizzato per il movimento della camera
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input ricevuto da tastiera, tasti WASD
        // -----
        processInput(window);

        // decido il  colore dello sfondo tramite glClearColor 
        glClearColor(0.1, 0.1f, 0.1f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //pulizia buffer

        // attivo gli shader
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position); // view basata sulla posizione della camera
        lightingShader.setFloat("material.shininess", 32.0f); // shiness impostato a 32, determina riflesso della luce sull'oggetto

   
        
        /*
        Qui vado a definire tutte le specifiche necessarie per le luci tramite variabili Uniform
        Ogni luce presenta una componente direzionale, una ambientale, uan diffusiva e una speculare
        Le lampadine presentano anche una posizione all'interno della scena e 3 componenti(costante,lineare,quadratica) utilizzate nel calcolo dell'attenuazione
        Lo spotlight prende la posizione della camera come riferimento
        */


       // // directional light
        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.09);
        lightingShader.setFloat("pointLights[0].quadratic", 0.032);
        // point light 2
        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[1].constant", 1.0f);
        lightingShader.setFloat("pointLights[1].linear", 0.09);
        lightingShader.setFloat("pointLights[1].quadratic", 0.032);
        // point light 3
        lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[2].constant", 1.0f);
        lightingShader.setFloat("pointLights[2].linear", 0.09);
        lightingShader.setFloat("pointLights[2].quadratic", 0.032);


        // point light 4 le coordinate y e z cambiano in base al tempo siccome la luce ruota
        float light3_mov_y = pointLightPositions[3].y + cos(glfwGetTime()) * 2.5;
        float light3_mov_z = pointLightPositions[3].z + sin(glfwGetTime()) * 2.0 ;

        lightingShader.setVec3("pointLights[3].position", pointLightPositions[3].x, light3_mov_y, light3_mov_z );
        lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[3].constant", 1.0f);
        lightingShader.setFloat("pointLights[3].linear", 0.09);
        lightingShader.setFloat("pointLights[3].quadratic", 0.032);
        // spotLight
        lightingShader.setVec3("spotLight.position", camera.Position );
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09);
        lightingShader.setFloat("spotLight.quadratic", 0.032);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(8.0f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(9.0f)));
        
     
      
       
      


        //trasformazione view/projection 
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);


        //trasformazione world 
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        //render del backpack
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); //modello al centro della scena
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// rimpicciolisco il modello
        lightingShader.setMat4("model", model);
        ourModel.Draw(modelShader);

         //draw dell lampadine nella scena
         modelShader.use();
         modelShader.setMat4("projection", projection);
         modelShader.setMat4("view", view);
        //la quarta luce dovra' muoversi attorno allo zaino 
         glBindVertexArray(lightCubeVAO);
         for (unsigned int i = 0; i < 4; i++)
         {
             model = glm::mat4(1.0f);

             if (i == 3)
             {
                 model = glm::translate(model, glm::vec3(pointLightPositions[3].x, light3_mov_y, light3_mov_z) );
             }
             else   
                model = glm::translate(model, pointLightPositions[i]);

             model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
             modelShader.setMat4("model", model);
             glDrawArrays(GL_TRIANGLES, 0, 36);
         }
         

        
        glfwSwapBuffers(window);  //swap del color buffer
        glfwPollEvents();   //controlla se sono stati ricevuti input da tastiera o mouse
    }


    //termina e de-alloca le risorse glfw
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// processa gli input che arrivano da tastiera, muove la camera in base al tasto premuto
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{

   
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) //W camera in avanti
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) //S camera indietro
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) //A camera a sinistra
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) //D camera a destra
        camera.ProcessKeyboard(RIGHT, deltaTime);
    


}



// callback che si occupa della gestione della window
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // la viewport deve essere uguale alla dimensione della window 
    glViewport(0, 0, width, height);
}

// callback per il movimento del mouse
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // invertite siccome y va dal basso verso l'alto

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}



//call back per la rotellina del mouse 
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


