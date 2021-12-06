/*
L'esericitazione presenta al suo interno 2 modelli caricati da assimp (backpack e dancing_vampire)
che vengono illuminati da diverse luci ed il punto di vista viene dato da una camera mobile.
Le luci in totale sono 9:
    1 directional light
    1 spot light posizionata nello stesso punto della camera e ne imita il movimento (movimento del mouse e tasti WASD)
    7 point light:
        2 illuminano il modello backpack
        2 illuminano il modello dancing_vampire
        1 ruota attorno al modello backpack
        1 ruota attorno al modello dancing_vampire
        1 posizionata nel mezzo e pu� muoversi sull'asse z tramite scroll della rotellina del mouse e sugli assi x ed y con le frecce
          Inoltre pu� far anche variare la componente di illuminazione ambientale
            R: varia il rosso
            G: varia il verde
            B: varia il blu
Durante il caricamento degli shader � possibile scegliere di caricare solo lo shader con le luci multiple o quello con anche il tangent space.
*/


#include <glad/glad.h>
#include <GLFW/glfw3.h>
//shader
#include <learnopengl/shader_m.h>

//Libreria per il caricamento dell'immagine
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//glm -> funzioni matematiche
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//camera
#include <learnopengl/camera.h>


//mesh
#include <learnopengl/model.h>


#include <iostream>

//Callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height); //finestra
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window); //input
unsigned int loadTexture(const char* path); //funzione che carica la texture

//Dimensioni finestra
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//Camera
Camera camera(glm::vec3(5.5f, 0.0f, 15.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//Timing
float deltaTime = 0.0f; //tempo fra il frame corrente e l'ultimo frame
float lastFrame = 0.0f;

//Lighting
//Posizione delle point light
glm::vec3 pointLightPositions[] = {
    //backpack
    glm::vec3(-1.3f,  -1.5f,  2.0f), //davanti
    glm::vec3(1.5f, 2.2f, -1.5f), //dietro in basso
    glm::vec3(4.0f,  0.0f, -1.5f), //camera mossa dall'input dell'utente
    glm::vec3(0.0f,  0.8f, -0.2f), //ruota attorno a backpack
    //vampire
    glm::vec3(8.8f, 0.8f, 0.0f), //ruota attorno a vampire
    glm::vec3(12.6f, -2.1f, 2.4f), //davanti
    glm::vec3(6.5f, 2.3f, -2.4f) //dietro
};

//Componente ambientale
glm::vec3 lightAmbientColor = glm::vec3(0.0f, 0.0f, 0.0f);


int main() {
    //Procedura per instanziare la GLFW window (creare la finestra)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    //Creiamo la finestra
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);//creaiamo il contesto
    //Chiamata alla funzione per la gestione della dimensione della window
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //Funzioni per l'utilizzo del mouse
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);


    //Controllo sul caricamento di glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //Abilito il deph test (z-buffer per capire la profondit�)
    //Se non abilito la profondit� e la cancellazione del buffer con GL_DEPTH_BUFFER_BIT nel render loop non ho profondit�
    //Viene renderizzato quindi come elemento in primo piano quello che c'� ll'interno del buffer in quel momento
    glEnable(GL_DEPTH_TEST);

    //Abilito la cattura del mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    //Flip della texture
    stbi_set_flip_vertically_on_load(true);


    //Shader
    Shader ourShader("model_Load.vs", "model_Load.fs");
    //Illuminazione multipla
    //Shader lightShader("Lights.vs", "Lights.fs");
    //Illuminazione multipla nello spazio tangente
    Shader lightShader("Lights_Tangent.vs", "Lights_Tangent.fs");
    

    //Caricamento del modello
    //Assimp si occupa anche di caricare le texture associate al modello perci� non � necessario caricarle
    //Model backpack("D:/Prog_Grafica Esercizi/Mesh/backpack/backpack.obj");
    Model backpack("C:/Users/Baroni Alberto/Desktop/sguola/Magistrale/grafica/backpack/backpack.obj");
    Model vampire("C:/Users/Baroni Alberto/Desktop/sguola/Magistrale/grafica/LearnOpenGL-master/resources/objects/vampire/dancing_vampire.dae");



    //Trattamento dei dati -> cubo: 36 vertici e 6 facce
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


    //Cube 
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


    //Light 
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    //posizione
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



    //Booleani per il movimento lungo l'asse y
    bool backpackUp = true;
    bool vampireUp = true;
    //Render loop per la window
    //Verifichiamo che la finestra sia aperta
    while (!glfwWindowShouldClose(window)) {
        //Input per la chiusura
        processInput(window);

        //Colore di sfondo
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
         //Cancelliamo il colore del buffer e lo ridefiniamo con quello inserito in glClearColor 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Pulisco anche il buffer

        //Calcolo frame
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //Attivazione dello shader
        lightShader.use();
        lightShader.setVec3("viewPos", camera.Position); //posizione della camera

        //Propriet� dei materiali
        lightShader.setFloat("material.shininess", 32.0f);


        //Definizione degli uniform passati per le luci
        //Directional light
        lightShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        //Point light 1
        lightShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        lightShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        lightShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightShader.setFloat("pointLights[0].constant", 1.0f);
        lightShader.setFloat("pointLights[0].linear", 0.09);
        lightShader.setFloat("pointLights[0].quadratic", 0.032);
        //Point light 2
        lightShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        lightShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        lightShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightShader.setFloat("pointLights[1].constant", 1.0f);
        lightShader.setFloat("pointLights[1].linear", 0.09);
        lightShader.setFloat("pointLights[1].quadratic", 0.032);
        //Point light 3
        lightShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        lightShader.setVec3("pointLights[2].ambient", lightAmbientColor.x, lightAmbientColor.y, lightAmbientColor.z);
        lightShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        lightShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        lightShader.setFloat("pointLights[2].constant", 1.0f);
        lightShader.setFloat("pointLights[2].linear", 0.09);
        lightShader.setFloat("pointLights[2].quadratic", 0.032);
        //Point light 4
        lightShader.setVec3("pointLights[3].position", pointLightPositions[3].x + sin(glfwGetTime())*2.0, pointLightPositions[3].y, pointLightPositions[3].z + cos(glfwGetTime())*1.5);
        lightShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        lightShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        lightShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        lightShader.setFloat("pointLights[3].constant", 1.0f);
        lightShader.setFloat("pointLights[3].linear", 0.09);
        lightShader.setFloat("pointLights[3].quadratic", 0.032);
        //Point light 5
        lightShader.setVec3("pointLights[4].position", pointLightPositions[4].x + sin(glfwGetTime()) * 2.5, pointLightPositions[4].y, pointLightPositions[4].z + cos(glfwGetTime()) * 2.5);
        lightShader.setVec3("pointLights[4].ambient", 0.05f, 0.05f, 0.05f);
        lightShader.setVec3("pointLights[4].diffuse", 0.8f, 0.8f, 0.8f);
        lightShader.setVec3("pointLights[4].specular", 1.0f, 1.0f, 1.0f);
        lightShader.setFloat("pointLights[4].constant", 1.0f);
        lightShader.setFloat("pointLights[4].linear", 0.09);
        lightShader.setFloat("pointLights[4].quadratic", 0.032);
        //Point light 6
        lightShader.setVec3("pointLights[5].position", pointLightPositions[5]);
        lightShader.setVec3("pointLights[5].ambient", 0.05f, 0.05f, 0.05f);
        lightShader.setVec3("pointLights[5].diffuse", 0.8f, 0.8f, 0.8f);
        lightShader.setVec3("pointLights[5].specular", 1.0f, 1.0f, 1.0f);
        lightShader.setFloat("pointLights[5].constant", 1.0f);
        lightShader.setFloat("pointLights[5].linear", 0.09);
        lightShader.setFloat("pointLights[5].quadratic", 0.032);
        //Point light 7
        lightShader.setVec3("pointLights[6].position", pointLightPositions[6]);
        lightShader.setVec3("pointLights[6].ambient", 0.05f, 0.05f, 0.05f);
        lightShader.setVec3("pointLights[6].diffuse", 0.8f, 0.8f, 0.8f);
        lightShader.setVec3("pointLights[6].specular", 1.0f, 1.0f, 1.0f);
        lightShader.setFloat("pointLights[6].constant", 1.0f);
        lightShader.setFloat("pointLights[6].linear", 0.09);
        lightShader.setFloat("pointLights[6].quadratic", 0.032);

        //SpotLight
        lightShader.setVec3("spotLight.position", camera.Position.x, camera.Position.y, camera.Position.z);
        lightShader.setVec3("spotLight.direction", camera.Front);
        lightShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightShader.setFloat("spotLight.constant", 1.0f);
        lightShader.setFloat("spotLight.linear", 0.09);
        lightShader.setFloat("spotLight.quadratic", 0.032);
        lightShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(7.5)));
        lightShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(10.0f)));


        // view/projection 
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        //render di backpack
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); //traslazione del modello
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	//scalatura del modello
        lightShader.setMat4("model", model);
        backpack.Draw(ourShader);

        //render di vampire
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(9.0f, -5.0f, 0.0f)); //traslazione del modello
        model = glm::scale(model, glm::vec3(0.045f, 0.045f, 0.045f)); //scalatura del modello
        lightShader.setMat4("model", model);
        vampire.Draw(ourShader);


        //disegno le luci -> piccoli cubi
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        //Disegno le luci
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 7; i++)
        {
            model = glm::mat4(1.0f);
            //Movimento luce che ruota attorno a backpack
            if (i == 3) 
            {
                model = glm::translate(model, glm::vec3(pointLightPositions[i].x + sin(glfwGetTime()) * 2.0, pointLightPositions[i].y, pointLightPositions[i].z + cos(glfwGetTime()) * 1.5)); //posizioni delle luci
                if (backpackUp == true) //vado in gi� perch� la luce � partita dall'alto
                {
                    if (pointLightPositions[i].y > -2.0f)
                        pointLightPositions[i].y -= 0.0005f;
                    else
                        backpackUp = false;
                }
                else if (backpackUp == false) //vado in su perch� la luce � partita dal basso
                {
                    if (pointLightPositions[i].y < 1.5f)
                        pointLightPositions[i].y += 0.0005f;
                    else
                        backpackUp = true;
                }
            }
            if (i == 4)
            {
                model = glm::translate(model, glm::vec3(pointLightPositions[i].x + sin(glfwGetTime()) * 2.5, pointLightPositions[i].y, pointLightPositions[i].z + cos(glfwGetTime()) * 2.5)); //posizioni delle luci
                if (vampireUp == true) //vado in gi� perch� la luce � partita dall'alto
                {
                    if (pointLightPositions[i].y > -4.0f)
                        pointLightPositions[i].y -= 0.0005f;
                    else
                        vampireUp = false;
                }
                else if (vampireUp == false) //vado in su perch� la luce � partita dal basso
                {
                    if (pointLightPositions[i].y < 3.5f)
                        pointLightPositions[i].y += 0.0005f;
                    else
                        vampireUp = true;
                }
            }
            else
                model = glm::translate(model, pointLightPositions[i]); //posizioni delle luci
            model = glm::scale(model, glm::vec3(0.2f)); //dim. luce
            ourShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        glfwSwapBuffers(window); //Mando nella window quello che c'� nel buffer (color buffer)
        glfwPollEvents(); //Verifico che non ci siano altri eventi
    }

    //Indichiamo che abbiamo finito di utilizzare le funzioni di glfw e usciamo dal programma
    glfwTerminate(); //ripristiniamo
    return 0;
}

//Processa gli input da tastiera
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //Movimento della telecamera tramite WASD in relazione al tempo
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, (deltaTime * 2.5f));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, (deltaTime * 2.5f));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, (deltaTime * 2.5f));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, (deltaTime * 2.5f));

    //Movimento della luce sugli assi x e y con velocit� fissata
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pointLightPositions[2] = glm::vec3(pointLightPositions[2].x, pointLightPositions[2].y+0.01, pointLightPositions[2].z);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pointLightPositions[2] = glm::vec3(pointLightPositions[2].x, pointLightPositions[2].y - 0.01, pointLightPositions[2].z);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        pointLightPositions[2] = glm::vec3(pointLightPositions[2].x - 0.01, pointLightPositions[2].y, pointLightPositions[2].z);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        pointLightPositions[2] = glm::vec3(pointLightPositions[2].x + 0.01, pointLightPositions[2].y, pointLightPositions[2].z);

    //Cambio il colore della componente ambientale della seconda camera
    //Red
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) 
    {
        if (lightAmbientColor.x < 1.0f)
            lightAmbientColor.x += 0.001f;
        else
            lightAmbientColor.x = 0.0f;
    }
    //Green
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        if (lightAmbientColor.y < 1.0f)
            lightAmbientColor.y += 0.001f;
        else
            lightAmbientColor.y = 0.0f;
    }
    //Blue
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        if (lightAmbientColor.z < 1.0f)
            lightAmbientColor.z += 0.001f;
        else
            lightAmbientColor.z = 0.0f;
    }
}



//Definizione della callback che gestisce la dimensione della window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


//Processa i movimenti del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    //Se muovo il mouse aggiorno la posizione
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    //calcolo lo spostamento
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; //Al contrario perch� le coordinate y vanno dal basso verso l'alto

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

//Funzione per lo scroll della rotellina del mouse (fa muovere la luce sull'asse z)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    pointLightPositions[2] = glm::vec3(pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z - yoffset);
}

//Funzione che carica una texture
unsigned int loadTexture(char const* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}