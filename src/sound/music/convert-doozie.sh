 #ffmpeg -i The_Doozie.m4a \
    #-acodec libvorbis -aq 4 -vn -ac 2 \
    #-map_metadata 0 \
    #The_Doozie.ogg

 ffmpeg -i Chance_of_Rain.m4a \
    -acodec libvorbis -aq 4 -vn -ac 2 \
    -map_metadata 0 \
    Chance_of_Rain.ogg
