//Fragment Shader lighting
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse; // componente diffusiva
    sampler2D specular; //componente speculare
    sampler2D normal;
    float shininess; //shininess del materiale
};

//attributi passati dal vertex come input
in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    //vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    mat3 TBN;
} fs_in;


//definisco le strutture utilizzate dal fragment shader per il lighting
//------------------------------------------------------------

//Directional light: la direzione della luce rimane sempre la stessa 
struct DirLight {
    vec3 direction; //direzione
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//Point light
struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//Spot light: emette luce solo nel raggio definito dallo spotlight
struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

//Numero delle point light
#define NR_POINT_LIGHTS 4


//variabili uniform
uniform vec3 viewPos;
uniform DirLight dirLight;  
uniform PointLight pointLights[NR_POINT_LIGHTS]; //array delle point lights
uniform SpotLight spotLight;
uniform Material material;

//Funzioni per il calcolo delle 3 tipologie di luce
//vengono utilizzate delle funzioni per non appesantire il main
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    //Tangent space
    vec3 normal = texture(material.normal, fs_in.TexCoords).rgb; //calcolo la normale nel tangent space
    normal = normalize(normal * 2.0 - 1.0); //da [0,1] a [-1,1]


    vec3 norm = normalize(fs_in.Normal); //normale per il diffuse lighting
    vec3 tanViewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos); //aggiungo la tangente
    vec3 viewDir = normalize(viewPos - fs_in.FragPos); 
    

    //calcolo il contributo di ogni luce e lo sommo in result, in result ci sara' il colore finale del Fragment
    //-------------------------------------------------------
    //Directional light
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    //Point light-> calcolo il contributo di ogni lampadina 
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], normal, fs_in.TangentFragPos, tanViewDir);    
    //Spot light
    result += CalcSpotLight(spotLight, norm, fs_in.FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
}

//Calcolo del colore sulla Directional Light
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction); //negativa siccome va dal frammento verso la sorgente di luce
    //Diffuse shading 
    float diff = max(dot(normal, lightDir), 0.0); 
    //Specular shading 
    vec3 reflectDir = reflect(-lightDir, normal); //negativo, prende la direzione opposta
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); //ottengo un coseno che blocco a 0 ed elevo per material.shininess
    //Combino i risultati
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    return (ambient + diffuse + specular);
}

//Calcolo del colore sulla point light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    //posizione della luce x la matrice TBN
    vec3 lightDir = normalize((fs_in.TBN * light.position) - fragPos); //Direzione del raggio associato ad ogni frammento
    //Diffuse shading -
    float diff = max(dot(normal, lightDir), 0.0); 
    //Specular shading 
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); //formula luce speculare
    // Attenuazione
    float distance = length((fs_in.TBN * light.position) - fragPos); //distanza dalla sorgente di luce
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); //formula per l'attenuazione 
    //Combino i risultati
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    //moltiplico le componenti per l'attenuazione
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

//Calcolo del colore sullo spot light
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{                        
    vec3 lightDir = normalize(light.position - fragPos); 
    //Diffuse shading 
    float diff = max(dot(normal, lightDir), 0.0); 
    //Specular shading 
    vec3 reflectDir = reflect(-lightDir, normal); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
    // Attenuazione
    float distance = length(light.position - fragPos); //distanza dalla sorgente di luce
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); //formula per l'attenuazione
   //calcoli per avere i bordi della spotlight smoothed
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    //Combino i risultati
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    //moltiplico le componenti per attenuazione e intensità
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
