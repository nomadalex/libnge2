#ifndef CANIMATION_H_
#define CANIMATION_H_
#include "libnge2.h"
extern int game_quit;

enum{
		ANI_RIGHT = 0,
		ANI_LEFT ,
		ANI_UP,
		ANI_DOWN
};

typedef struct  {
	float x;
	float y;
	float w;
	float h;
}rect;

class CAnimation
{
public:
	
	CAnimation(const char* filename);
	~CAnimation();
    //Moves the stick figure
    void Update();    
    void Input();
    //Shows the stick figure
    void Show();
    static float velocity_x;
	static float velocity_y;
protected:
	void set_clip();
	void InputInit();
	static void btn_down(int keycode);
	static void btn_up(int keycode);
private:
	   
    //Its current frame
    int frame;

	float offset_x;
	float offset_y;
	//Its animation status
    int status;
	//The areas of the sprite sheet
	rect clipsRight[ 4 ];
	rect clipsLeft[ 4 ];
	rect clipsUp[ 4 ];
	rect clipsDown[ 4 ];
	image_p resouce;
	//Its rate of movement
	
};


#endif