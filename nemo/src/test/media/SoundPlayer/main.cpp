#include <stdio.h>
#include "Sound.h"
#include "SoundPlayer.h"

void playsound(char *path) {
   BSound *sound;
   BSoundPlayer player;
   //entry_ref ref;
   //BEntry entry(path, true);
   BSoundPlayer::play_id id;

   //if (entry.InitCheck()) == B_OK) {
      //if (entry.GetRef(&ref) == B_OK) {
         //sound = new BSound(&ref);
         if (sound->InitCheck() == B_OK) {
            player.Start();
	    printf("Initializing.... \n");
            player.SetVolume(1.0);
	    printf("Setting Volume to 1.0.... \n");
            id = player.StartPlaying(sound);
	    printf("Staring Play .... Is now playing ??\n");
            sound->ReleaseRef();
            player.WaitForSound(id);
	    printf("I think it stopped (or never played)\n");
        // }
      }
  // }
} 


int main()
{



	playsound("test.wav");
/*	BSoundPlayer player;
	player.Start();
	
	play_id id = player->StartPlaying(sound);

	
	if (player->IsPlaying(id)) 
	{
		printf("The sound is still playing ");
  	} 
	
	//player->StopSound(id); 
	
	player->WaitForSound(id); 
	player.Stop();  */
	
	printf("Successfull or not??.... \n");
	return 0;
}


