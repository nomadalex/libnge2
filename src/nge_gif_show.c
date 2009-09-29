#include "nge_gif_show.h"


void GifAnimationToScreen(gif_desc_p pgif,float x,float y)
{
	if(pgif->gif_show_status == GIF_SHOW_INIT){
		pgif->gif_inner_timer->start(pgif->gif_inner_timer);
		pgif->gif_current_chains = pgif->gif_image_chains;
		pgif->gif_last_ticks = 0;
		pgif->gif_show_status = GIF_SHOW_PLAY;
		pgif->gif_delay = pgif->gif_current_chains->frame_delay;
	}
	if(pgif->gif_show_status!=GIF_SHOW_STOP){
		pgif->gif_current_ticks = pgif->gif_inner_timer->get_ticks(pgif->gif_inner_timer);
		if(pgif->gif_current_ticks-pgif->gif_last_ticks >= pgif->gif_delay*10){
			pgif->gif_last_ticks = pgif->gif_current_ticks;
			pgif->gif_current_frame = pgif->gif_current_chains->pimage;
			pgif->gif_delay = pgif->gif_current_chains->frame_delay;
			pgif->gif_current_chains = pgif->gif_current_chains->next;
			if(pgif->gif_current_chains == NULL){
				if(pgif->gif_loopcount>10){
					pgif->gif_current_chains = pgif->gif_image_chains;
					pgif->gif_delay = pgif->gif_current_chains->frame_delay;
				}
				else
					pgif->gif_show_status=GIF_SHOW_STOP;
			}
		}
	}
	if(pgif->gif_current_frame)
		ImageToScreen(pgif->gif_current_frame,x,y);	
}

void RenderGifAnimation(gif_desc_p pgif,float sx ,float sy ,float sw ,float sh ,float dx ,float dy ,float xscale  ,float yscale ,float angle ,int mask)
{
	if(pgif->gif_show_status == GIF_SHOW_INIT){
		pgif->gif_inner_timer->start(pgif->gif_inner_timer);
		pgif->gif_current_chains = pgif->gif_image_chains;
		pgif->gif_last_ticks = 0;
		pgif->gif_show_status = GIF_SHOW_PLAY;
		pgif->gif_delay = pgif->gif_current_chains->frame_delay;
	}
	if(pgif->gif_show_status!=GIF_SHOW_STOP){
		pgif->gif_current_ticks = pgif->gif_inner_timer->get_ticks(pgif->gif_inner_timer);
		if(pgif->gif_current_ticks-pgif->gif_last_ticks >= pgif->gif_delay*10){
			pgif->gif_last_ticks = pgif->gif_current_ticks;
			pgif->gif_current_frame = pgif->gif_current_chains->pimage;
			pgif->gif_delay = pgif->gif_current_chains->frame_delay;
			pgif->gif_current_chains = pgif->gif_current_chains->next;
			if(pgif->gif_current_chains == NULL){
				if(pgif->gif_loopcount>10){
					pgif->gif_current_chains = pgif->gif_image_chains;
					pgif->gif_delay = pgif->gif_current_chains->frame_delay;
				}
				else
					pgif->gif_show_status=GIF_SHOW_STOP;
			}
		}
	}
	if(pgif->gif_current_frame)
		RenderQuad(pgif->gif_current_frame,sx,sy,sw,sh,dx,dy,xscale,yscale,angle,mask);	
}

void DrawGifAnimationMask(gif_desc_p pgif,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int mask)
{
	if(pgif->gif_show_status == GIF_SHOW_INIT){
		pgif->gif_inner_timer->start(pgif->gif_inner_timer);
		pgif->gif_current_chains = pgif->gif_image_chains;
		pgif->gif_last_ticks = 0;
		pgif->gif_show_status = GIF_SHOW_PLAY;
		pgif->gif_delay = pgif->gif_current_chains->frame_delay;
	}
	if(pgif->gif_show_status!=GIF_SHOW_STOP){
		pgif->gif_current_ticks = pgif->gif_inner_timer->get_ticks(pgif->gif_inner_timer);
		if(pgif->gif_current_ticks-pgif->gif_last_ticks >= pgif->gif_delay*10){
			pgif->gif_last_ticks = pgif->gif_current_ticks;
			pgif->gif_current_frame = pgif->gif_current_chains->pimage;
			pgif->gif_delay = pgif->gif_current_chains->frame_delay;
			pgif->gif_current_chains = pgif->gif_current_chains->next;
			if(pgif->gif_current_chains == NULL){
				if(pgif->gif_loopcount>10){
					pgif->gif_current_chains = pgif->gif_image_chains;
					pgif->gif_delay = pgif->gif_current_chains->frame_delay;
				}
				else
					pgif->gif_show_status=GIF_SHOW_STOP;
			}
		}
	}
	if(pgif->gif_current_frame)
		DrawImageMask(pgif->gif_current_frame,sx,sy,sw,sh,dx,dy,dw,dh,mask);	
}

void DrawGifAnimation(gif_desc_p pgif,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh)
{
	if(pgif->gif_show_status == GIF_SHOW_INIT){
		pgif->gif_inner_timer->start(pgif->gif_inner_timer);
		pgif->gif_current_chains = pgif->gif_image_chains;
		pgif->gif_last_ticks = 0;
		pgif->gif_show_status = GIF_SHOW_PLAY;
		pgif->gif_delay = pgif->gif_current_chains->frame_delay;
	}
	if(pgif->gif_show_status!=GIF_SHOW_STOP){
		pgif->gif_current_ticks = pgif->gif_inner_timer->get_ticks(pgif->gif_inner_timer);
		if(pgif->gif_current_ticks-pgif->gif_last_ticks >= pgif->gif_delay*10){
			pgif->gif_last_ticks = pgif->gif_current_ticks;
			pgif->gif_current_frame = pgif->gif_current_chains->pimage;
			pgif->gif_delay = pgif->gif_current_chains->frame_delay;
			pgif->gif_current_chains = pgif->gif_current_chains->next;
			if(pgif->gif_current_chains == NULL){
				if(pgif->gif_loopcount>10){
					pgif->gif_current_chains = pgif->gif_image_chains;
					pgif->gif_delay = pgif->gif_current_chains->frame_delay;
				}
				else
					pgif->gif_show_status=GIF_SHOW_STOP;
			}
		}
	}
	if(pgif->gif_current_frame)
		DrawImageMask(pgif->gif_current_frame,sx,sy,sw,sh,dx,dy,dw,dh,pgif->gif_current_frame->mask);
}