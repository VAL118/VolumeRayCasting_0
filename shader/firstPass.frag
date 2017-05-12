precision mediump int;
precision mediump float;

varying vec4 backColor;


//
varying vec3 worldSpaceCoords;

void main(void)
{
    //gl_FragColor = backColor;
    gl_FragColor = vec4( worldSpaceCoords.x , worldSpaceCoords.y, worldSpaceCoords.z, 1);
}
