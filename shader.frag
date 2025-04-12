#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 texCoord; // Coordenadas de textura

out vec4 FragColor;

uniform vec3 solDir; 
uniform vec3 solColor;
uniform float solIntensidad;

uniform int farosEncendidos;
uniform vec3 faroPos;
uniform vec3 faroColor;
uniform float faroIntensidad;
uniform vec3 faroDireccion;
uniform float faroCorte;
uniform float faroBordeSuave;

uniform vec3 ambientColor;
uniform float ambientIntensidad;

uniform vec3 objectColor;
uniform vec3 viewPos; // Posición de la cámara

uniform sampler2D texturaSampler;    

void main()
{
    // **Luz Ambiental**
    vec3 ambient = ambientIntensidad * ambientColor;
    
    // **Luz Direccional del Sol**
    vec3 norm = normalize(Normal);
    vec3 solDirNorm = normalize(solDir); 
    float diff = max(dot(norm, solDirNorm), 0.0);
    vec3 diffuse = solIntensidad * diff * solColor;
    
    // **Luz Especular del Sol**
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectSolDir = reflect(-solDirNorm, norm);
    float spec = pow(max(dot(viewDir, reflectSolDir), 0.0), 32);
    vec3 specular = solIntensidad * 0.1 * spec * solColor;
    
    // **Luz del Faro (Spotlight)**
    vec3 diffuseFaro = vec3(0.0);
    if (farosEncendidos == 1)
    {    
        vec3 faroDir = normalize(faroPos - FragPos);
        float theta = dot(faroDir, normalize(-faroDireccion)); 
        float epsilon = faroBordeSuave - faroCorte;  
        float spotFactor = clamp((theta - faroCorte) / epsilon, 0.0, 1.0);  
        float diffFaro = max(dot(norm, faroDir), 0.0);
        diffuseFaro = faroIntensidad * diffFaro * faroColor * spotFactor;
    }

    // **Carga de Textura con Canal Alfa (PNG)**
    vec4 textureColor = texture(texturaSampler, texCoord);  // Cargar con RGBA
    
    FragColor  = vec4((ambient + diffuse + specular + diffuseFaro) * textureColor.rgb, textureColor.a);
    }

