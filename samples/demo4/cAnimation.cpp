#include "cAnimation.h"
#define ANI_WIDTH  70
#define ANI_HEIGHT 124

#define ANI_VELOCITY 1.5

float CAnimation::velocity_x = 0;
float CAnimation::velocity_y = 0;

CAnimation::CAnimation(const char* filename)
{
	resouce = image_load(filename,DISPLAY_PIXEL_FORMAT_8888,1);
	set_clip();

	//Initialize movement variables
    offset_x = 0;
	offset_y = 120;
    velocity_x = 0;
    velocity_y = 0;
    //Initialize animation variables
    frame = 0;
    status = ANI_RIGHT;
	InitInput(btn_down,btn_up,1);
}

CAnimation::~CAnimation()
{
	image_free(resouce); 
}

void CAnimation::btn_down(int keycode)
{
	switch(keycode)
    {
	case PSP_BUTTON_UP:
		velocity_y -= ANI_VELOCITY;
		break;
	case PSP_BUTTON_DOWN:
		velocity_y += ANI_VELOCITY;
		break;
	case PSP_BUTTON_LEFT:
		velocity_x -= ANI_VELOCITY;
		break;
	case PSP_BUTTON_RIGHT:
		velocity_x += ANI_VELOCITY; 
		break;
	case PSP_BUTTON_TRIANGLE:
		break;
	case PSP_BUTTON_CIRCLE:
		break;
	case PSP_BUTTON_CROSS:
		break;
    case PSP_BUTTON_SQUARE:
		break;
	case PSP_BUTTON_SELECT:
		game_quit = 1;
		break;
	case PSP_BUTTON_START:
		game_quit = 1;
		break;
    }
}

void CAnimation::btn_up(int keycode)
{
	switch(keycode)
    {
	case PSP_BUTTON_UP:
		velocity_y += ANI_VELOCITY;
		break;
	case PSP_BUTTON_DOWN: 
		velocity_y -= ANI_VELOCITY;
		break;
	case PSP_BUTTON_LEFT:
		velocity_x += ANI_VELOCITY;
		break;
	case PSP_BUTTON_RIGHT:
		velocity_x -= ANI_VELOCITY; 
		break;
	case PSP_BUTTON_TRIANGLE:
		break;
	case PSP_BUTTON_CIRCLE:
		break;
	case PSP_BUTTON_CROSS:
		break;
    case PSP_BUTTON_SQUARE:
		break;
	case PSP_BUTTON_SELECT:
		game_quit = 1;
		break;
	case PSP_BUTTON_START:
		game_quit = 1;
		break;
    }
}


void CAnimation::Update()
{
	//Move
    offset_x += velocity_x;
    offset_y += velocity_y;
    //Keep the stick figure in bounds
    if( ( offset_x < 0 ) || ( offset_x + ANI_WIDTH > SCREEN_WIDTH ) )
    {
        offset_x -= velocity_x;    
    }
	if( ( offset_y < 0 ) || ( offset_y + ANI_HEIGHT > SCREEN_HEIGHT ) )
    {
        offset_y -= velocity_y;    
    }
}

void CAnimation::Input()
{
	InputProc();
}

void CAnimation::Show()
{
	static int step=0;
    //If Foo is moving left
    if( velocity_x < 0 )
    {
        //Set the animation to left
        status = ANI_LEFT;
        
        //Move to the next frame in the animation
        if(step>6){
			step=0;
        //Move to the next frame in the animation
        frame++;
		}
		step++;
    }
    //If Foo is moving right
    else if( velocity_x > 0 )
    {
        //Set the animation to right
        status = ANI_RIGHT;
        if(step>6){
			step=0;
        //Move to the next frame in the animation
        frame++;
		}
		step++;
    }
    
	
	if( velocity_y < 0 )
    {
        //Set the animation to left
        status = ANI_UP;
        
        //Move to the next frame in the animation
        if(step>6){
			step=0;
        //Move to the next frame in the animation
        frame++;
		}
		step++;
    }
    //If Foo is moving right
    else if( velocity_y > 0 )
    {
        //Set the animation to right
        status = ANI_DOWN;
        if(step>6){
			step=0;
        //Move to the next frame in the animation
        frame++;
		}
		step++;
    }
    

    
    //Loop the animation
    if( frame >= 4 )
    {
        frame = 0;
    }
    
    //Show the stick figure
    if( status == ANI_RIGHT )
    {
        //apply_surface( offSet, SCREEN_HEIGHT - FOO_HEIGHT, foo, screen, &clipsRight[ frame ] );
		
		RenderQuad(resouce,clipsRight[ frame ].x,clipsRight[ frame ].y,clipsRight[ frame].w,clipsRight[frame].h,offset_x,offset_y,0.6,0.6,0,resouce->mask);
		
    }
    else if( status == ANI_LEFT )
    {
        //apply_surface( offSet, SCREEN_HEIGHT - FOO_HEIGHT, foo, screen, &clipsLeft[ frame ] );
		RenderQuad(resouce,clipsLeft[ frame ].x,clipsLeft[ frame ].y,clipsLeft[ frame ].w,clipsLeft[ frame ].h,offset_x,offset_y,0.6,0.6,0,resouce->mask);
   }
	else if( status == ANI_UP )
    {
        //apply_surface( offSet, SCREEN_HEIGHT - FOO_HEIGHT, foo, screen, &clipsLeft[ frame ] );
		
		RenderQuad(resouce,clipsUp[ frame ].x,clipsUp[ frame ].y,clipsUp[ frame ].w,clipsUp[ frame ].h,offset_x,offset_y,0.6,0.6,0,resouce->mask);
		
    }
		else if( status == ANI_DOWN )
    {
        //apply_surface( offSet, SCREEN_HEIGHT - FOO_HEIGHT, foo, screen, &clipsLeft[ frame ] );
		
		RenderQuad(resouce,clipsDown[ frame ].x,clipsDown[ frame ].y,clipsDown[ frame ].w,clipsDown[ frame ].h,offset_x,offset_y,0.6,0.6,0,resouce->mask);
		
    }

}



void CAnimation::set_clip()
{
	//Clip the sprites
	//down
    clipsDown[ 0 ].x = 0;
    clipsDown[ 0 ].y = 0;
    clipsDown[ 0 ].w = ANI_WIDTH;
    clipsDown[ 0 ].h = ANI_HEIGHT;
    
    clipsDown[ 1 ].x = ANI_WIDTH;
    clipsDown[ 1 ].y = 0;
    clipsDown[ 1 ].w = ANI_WIDTH;
    clipsDown[ 1 ].h = ANI_HEIGHT;
    
    clipsDown[ 2 ].x = ANI_WIDTH * 2;
    clipsDown[ 2 ].y = 0;
    clipsDown[ 2 ].w = ANI_WIDTH;
    clipsDown[ 2 ].h = ANI_HEIGHT;
    
    clipsDown[ 3 ].x = ANI_WIDTH * 3;
    clipsDown[ 3 ].y = 0;
    clipsDown[ 3 ].w = ANI_WIDTH;
    clipsDown[ 3 ].h = ANI_HEIGHT;
    //left
    clipsLeft[ 0 ].x = 0;
    clipsLeft[ 0 ].y = ANI_HEIGHT;
    clipsLeft[ 0 ].w = ANI_WIDTH;
    clipsLeft[ 0 ].h = ANI_HEIGHT;
    
    clipsLeft[ 1 ].x = ANI_WIDTH;
    clipsLeft[ 1 ].y = ANI_HEIGHT;
    clipsLeft[ 1 ].w = ANI_WIDTH;
    clipsLeft[ 1 ].h = ANI_HEIGHT;
    
    clipsLeft[ 2 ].x = ANI_WIDTH * 2;
    clipsLeft[ 2 ].y = ANI_HEIGHT;
    clipsLeft[ 2 ].w = ANI_WIDTH;
    clipsLeft[ 2 ].h = ANI_HEIGHT;
    
    clipsLeft[ 3 ].x = ANI_WIDTH * 3;
    clipsLeft[ 3 ].y = ANI_HEIGHT;
    clipsLeft[ 3 ].w = ANI_WIDTH;
    clipsLeft[ 3 ].h = ANI_HEIGHT;
	//right
	clipsRight[ 0 ].x = 0;
    clipsRight[ 0 ].y = ANI_HEIGHT*2;
    clipsRight[ 0 ].w = ANI_WIDTH;
    clipsRight[ 0 ].h = ANI_HEIGHT;
    
    clipsRight[ 1 ].x = ANI_WIDTH;
    clipsRight[ 1 ].y = ANI_HEIGHT*2;
    clipsRight[ 1 ].w = ANI_WIDTH;
    clipsRight[ 1 ].h = ANI_HEIGHT;
    
    clipsRight[ 2 ].x = ANI_WIDTH * 2;
    clipsRight[ 2 ].y = ANI_HEIGHT*2;
    clipsRight[ 2 ].w = ANI_WIDTH;
    clipsRight[ 2 ].h = ANI_HEIGHT;
    
    clipsRight[ 3 ].x = ANI_WIDTH * 3;
    clipsRight[ 3 ].y = ANI_HEIGHT*2;
    clipsRight[ 3 ].w = ANI_WIDTH;
    clipsRight[ 3 ].h = ANI_HEIGHT;
	//up
	clipsUp[ 0 ].x = 0;
    clipsUp[ 0 ].y = ANI_HEIGHT*3;
    clipsUp[ 0 ].w = ANI_WIDTH;
    clipsUp[ 0 ].h = ANI_HEIGHT;
    
    clipsUp[ 1 ].x = ANI_WIDTH;
    clipsUp[ 1 ].y = ANI_HEIGHT*3;
    clipsUp[ 1 ].w = ANI_WIDTH;
    clipsUp[ 1 ].h = ANI_HEIGHT;
    
    clipsUp[ 2 ].x = ANI_WIDTH * 2;
    clipsUp[ 2 ].y = ANI_HEIGHT*3;
    clipsUp[ 2 ].w = ANI_WIDTH;
    clipsUp[ 2 ].h = ANI_HEIGHT;
    
    clipsUp[ 3 ].x = ANI_WIDTH * 3;
    clipsUp[ 3 ].y = ANI_HEIGHT*3;
    clipsUp[ 3 ].w = ANI_WIDTH;
    clipsUp[ 3 ].h = ANI_HEIGHT;
}