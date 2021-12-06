#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse; //texture per la componente diffusiva
    sampler2D specular; //texture per la componente speculare
    sampler2D normal;
    float shininess;
}; 

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    //vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    mat3 TBN;
} fs_in;

//Quando vengono utilizzate pi� luci all'interno della scena si ha un vettore colore come output. Ogni luce che si trova all'interno della scena fornisce un contributoall'illuminazione e  
//questo contributo deve essere calcolato e poi sommato agli altri. La somma dei singoli contributi fornisce il valore finale del vettore colore
//Definiamo quindi la struttura di ogni luce che prender� parte alla scena e nel main costruiremo le funzioni che calcolano i singoli contributi

//Directional light: la direzione della luce rimane la stessa per ogni oggetto della scena
struct DirLight {
    vec3 direction; //non ho la posizione -> ho direttamente la direzione del raggio
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//Point light: viene introdotto un effetto di attenuazione
struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//Spot light: emette luce solo in una specifica direzione. Solo gli oggetti all'interno del raggio della luce sono colpiti ed illuminati dalla luce, mentre il resto della scena rimane scuro
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

//Numero delle point light che voglio avere nella scena
#define NR_POINT_LIGHTS 7



uniform vec3 viewPos;
uniform DirLight dirLight; //Luce direzionale passata come uniform
uniform PointLight pointLights[NR_POINT_LIGHTS]; //array di point light
uniform SpotLight spotLight;
uniform Material material;

//Funzioni 
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    //Tangent space
    vec3 normal = texture(material.normal, fs_in.TexCoords).rgb; //normale dalla normal map da range [0;1] a range [-1;+1]
    normal = normalize(normal * 2.0 - 1.0); //normale in tangent space 

    vec3 norm = normalize(fs_in.Normal); 
    vec3 tanViewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos); //aggiungo la tangente
    vec3 viewDir = normalize(viewPos - fs_in.FragPos); 
    

    //Viene calcolato il contributo per ogni lampada che � stata definita
    //All'interno del main vengono presi tutti i colori calcolati e vengono sommati per calcolare il colore finale
    //Directional light
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    //Point light-> calcolo il contributo di ogni lampada (in questo caso ho 4 point light)
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], normal, fs_in.TangentFragPos, tanViewDir);    
    //Spot light
    result += CalcSpotLight(spotLight, norm, fs_in.FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
}

//Calcolo del colore quando viene utilizzata una directional light
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction); //negativa perch� intesa dal frammento verso la sorgente di luce
    //Diffuse shading -> ottenuta con il prodotto scalare tra la normale e la direzione del raggio
    float diff = max(dot(normal, lightDir), 0.0); //clipping per i valori negativi
    //Specular shading -> necessaria anche la direzione di vista
    vec3 reflectDir = reflect(-lightDir, normal); //negativo perch� essendo speculare prendo la direzione opposta
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); //ottengo un coseno che blocco a 0 ed elevo per material.shininess
    //Combino i risultati
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    return (ambient + diffuse + specular);
}

//Calcolo del colore quando viene utilizzata una point light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    //Moltiplico la posizione della luce per la matrice TBN
    vec3 lightDir = normalize((fs_in.TBN * light.position) - fragPos); //Direzione del raggio associato ad ogni frammento
    //Diffuse shading -> ottenuta con il prodotto scalare tra la normale e la direzione del raggio
    float diff = max(dot(normal, lightDir), 0.0); //clipping per i valori negativi
    //Specular shading -> necessaria anche la direzione di vista
    vec3 reflectDir = reflect(-lightDir, normal); //negativo perch� essendo speculare prendo la direzione opposta
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); //ottengo un coseno che blocco a 0 ed elevo per material.shininess
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

//Calcolo del colore quando viene utilizzata una spot light
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{                            //Pos. luce      //Pos. frammento
    vec3 lightDir = normalize(light.position - fragPos); //Direzione del raggio associato ad ogni frammento
    //Diffuse shading -> ottenuta con il prodotto scalare tra la normale e la direzione del raggio
    float diff = max(dot(normal, lightDir), 0.0); //clipping per i valori negativi
    //Specular shading -> necessaria anche la direzione di vista
    vec3 reflectDir = reflect(-lightDir, normal); //negativo perch� essendo speculare prendo la dire
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); //ottengo un coseno che blocco a 0 ed elevo per material.shininess
    // Attenuazione
    float distance = length(light.position - fragPos); //distanza dalla sorgente di luce
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); //formula per l'attenuazione
    //Intensit� spotlight 
    //Calcoli per capire ci� che si trova all'interno del cono della spotlight
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    //Combino i risultati
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    //moltiplico le componenti per l'attenuazione e per l'intensit�
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

