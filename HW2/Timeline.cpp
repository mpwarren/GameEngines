Timeline::Timeline(Timeline* anchorParam, int64_t ticParam){
    anchor = anchorParam;
    tic = ticParam;
    startTime = anchor->getTime(); 
}

Timeline::Timeline(){
    startTime = 
}

int64_t Timeline::getTime(){
    return (anchor->getTime() - startTime) / tic;
}

void Timeline::pause(){

}

void Timeline::unpause(){

}

void Timeline::changeTic(int tic){

}

bool Timeline::isPaused(){

}