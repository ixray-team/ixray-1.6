float rayheight=1;
float oldray=1;
float2 offset=0;
float oldtex=1;
float texatray;
float yintersect;
int i=0;

while(i<MaxSteps+2)
{

    float texatray=dot(HeightMapChannel, Tex.SampleGrad(TexSampler,UV+offset,InDDX, InDDY));

    if (rayheight < texatray)
        {
            float xintersect = (oldray-oldtex)+(texatray-rayheight);
            xintersect=(texatray-rayheight)/xintersect;
            yintersect=(oldray*(xintersect))+(rayheight*(1-xintersect));
            offset-=(xintersect*UVDist);
            break;
        }

    oldray=rayheight;
    rayheight-=stepsize;
    offset+=UVDist;
    oldtex=texatray;

    i++;
}


float2 saveoffset=offset;
float shadow=1;
float dist=0;


texatray=dot(HeightMapChannel, Tex.SampleGrad(TexSampler,UV+offset,InDDX, InDDY))+0.01;
float finalrayz=yintersect;

rayheight=texatray;
float lightstepsize=1/ShadowSteps;

int j=0;
while(j<ShadowSteps)
    {
        if(rayheight < texatray)
            {
                shadow=0;
                break;
            }
        else
            {
                shadow=min(shadow,(rayheight-texatray)*k/dist);
            }

        oldray=rayheight;
        rayheight+=TangentLightVector.z*lightstepsize;

        offset+=TangentLightVector.xy*lightstepsize;
        oldtex=texatray;

        texatray=dot(HeightMapChannel, Tex.SampleGrad(TexSampler,UV+offset,InDDX, InDDY));
        dist+=lightstepsize;
        j++;
    }

//shadow = 1;
float4 finalout;
finalout.xy=saveoffset;
finalout.z=finalrayz;
finalout.w=shadow;
return finalout;
