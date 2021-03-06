#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform bool hdr;
uniform bool invert;
uniform bool greyScale;
uniform float exposure;

void main(){

    const float gamma = 2.2;
    vec3 hdrColor=texture(hdrBuffer,TexCoords).rgb;
    vec3 bloomColor=texture(bloomBlur,TexCoords).rgb;
    if(bloom){
        hdrColor+=bloomColor;
    }
    if(hdr){
        vec3 result = vec3(1.0) - exp(-hdrColor*exposure);
        result=pow(result,vec3(1.0/gamma));
        FragColor=vec4(result,1.0);
    }
    else if(invert){
        FragColor=vec4(vec3(1.0 - texture(hdrBuffer, TexCoords)),1.0);
    }
    else if(greyScale){
        FragColor=texture(hdrBuffer,TexCoords);
        float average = (FragColor.r + FragColor.g + FragColor.b)/3;
        FragColor=vec4(average,average,average,1.0);
    }
    else{
        vec3 result = pow(hdrColor,vec3(1.0/gamma));
        FragColor = vec4(result,1.0);
    }


}