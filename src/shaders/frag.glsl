#version 330 core

out vec4 FragColor;

uniform vec3 lightPos;
uniform sampler2D texture0;

in vec3 fragnormal;
in vec3 fragpos;
in vec2 fragtexturecoords;

void main(){
    vec3 lightColor = vec3(0.0, 0.5, 0.9);

    vec3 lightDir = normalize(lightPos - fragpos);
    vec3 norm = normalize(fragnormal);
    vec3 cor = vec3(0.5, 0.7, 0.45);
    vec3 ambient = 0.22 * vec3(1.0,1.0,1.0);
    vec3 diffuse = max(0.0, dot(norm, lightDir)) * lightColor;

    FragColor = vec4((ambient + diffuse), 1.0) * texture(texture0, fragtexturecoords);
    //FragColor = texture(texture0, fragtexturecoords);
}
