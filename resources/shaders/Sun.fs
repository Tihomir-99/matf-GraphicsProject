#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material {
    sampler2D texture_diffuse1;
};
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform Material material;


void main()
{
    FragColor = texture(material.texture_diffuse1,TexCoords);
    float brightness=dot(FragColor.rgb,vec3(0.2126,0.7152,0.0722));
    if (brightness > 1.0){
        BrightColor=vec4(FragColor.rgb,1.0);
    }else{
        BrightColor = vec4(0.0,0.0,0.0,1.0);
    }
}