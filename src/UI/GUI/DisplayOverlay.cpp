#include "DisplayOverlay.h"
#include "UI/MenuManager.h"

DisplayOverlay::DisplayOverlay() : DisplayItem("DisplayOverlay") {
    needsUpdate = false;
}

#ifdef ENABLE_DISPLAY_ANIMATIONS
void DisplayOverlay::draw(){
    bool animationEnded = false;
    if(animationID == ANIM_WAIT){
        long t = getTickTime();
        animationEnded = t > waitTime;
    }else if(animationID == ANIM_SWEEP_IN_LEFT){
        long t = getTickTime();
        t = t*t/100; //Ease in, faster out
        t *= sweepSpeed;
        t -= barWidth;  // So that the animation start behind the screen limits.
        // Draws the pointy triangles
        for(int i = 0; i < height; i+=barWidth*2-1){
            for(int j = 0; j <= barWidth; j++){
                canvas->drawFastHLine(t, i+j, j, animationColor);
                canvas->drawFastHLine(t, i+barWidth*2-j, j, animationColor);
            }
            canvas->fillRect(0,0, t,height, animationColor);
        }

        animationEnded = t>(width+barWidth+2);

    }else if(animationID == ANIM_SWEEP_OUT_LEFT){
        long t = getTickTime();
        t = t*t/100; //Ease in, faster out
        t *= sweepSpeed;
        t -= barWidth;  // So that the animation start behind the screen limits.
        // Draws the pointy triangles
        for(int i = 0; i < height; i+=barWidth*2-1){
            for(int j = 0; j <= barWidth; j++){
                canvas->drawFastHLine(width-t, i+j, j, animationColor);
                canvas->drawFastHLine(width-t, i+barWidth*2-j, j, animationColor);
            }
            canvas->fillRect(0,0, width-t,height, animationColor);
        }

        animationEnded = t>(width+barWidth+2);

    }else if(animationID == ANIM_SWEEP_IN_RIGHT){
        // Won't work with sprites
        /*canvas->setRotation(3); // This is what I like to call a little trickery!
        animationID = ANIM_SWEEP_IN;
        draw();*/

        long t = getTickTime();
        t = t*t/100; //Ease in, faster out
        t *= sweepSpeed;
        t -= barWidth;  // So that the animation start behind the screen limits.
        for(int i = barWidth; i < height+barWidth; i+=barWidth*2){
            for(int j = 0; j <= barWidth; j++){
                canvas->drawFastHLine(width-t+j, i+j, barWidth, animationColor);
                canvas->drawFastHLine(width-t+j, i-j, barWidth, animationColor);
            }
            canvas->fillRect(width-t+barWidth, 0, t,height, animationColor);
        }

        animationEnded = t>(width+barWidth+2);

    }else if(animationID == ANIM_CIRCLE){
        uint32_t t = getTickTime();
        t = t*t*t/10000; //Ease in, faster out
        t *= circleSpeed; 
        canvas->fillCircle(width/2, height/2, t, animationColor);
        animationEnded = t > diagonalRadius;
    }else if(animationID == ANIM_CIRCUMFERENCE){
        double theta = getTickTime()/1000.0*TWO_PI * plottingSpeed;
        canvas->fillCircle(width/2 + plottingRadius*cos(theta + HALF_PI), height/2 + plottingRadius*sin(theta + HALF_PI),
                         outerPincelStroke, animationColor);
        double r = plottingRadius*theta/TWO_PI;
        canvas->fillCircle(width/2 + r*cos(theta + HALF_PI), height/2 + r*sin(theta + HALF_PI),
                         innerPincelStroke, animationColor);
        animationEnded = theta >= TWO_PI;

    }else if(animationID == ANIM_EXCLAMATION){
        double theta = getTickTime()/1000.0*TWO_PI;
        theta += 7.0*PI/16.0;
        double r = -7.0*plottingRadius*cos(8.0*theta)/8.0;
        double c1 = height/2 + 7.0*plottingRadius/32.0;
        //This is part of a rose of 8 pethals. Is the bar of the exclamation.
        canvas->fillCircle(width/2 + r*cos(theta), c1 + r*sin(theta),
                            innerPincelStroke, animationColor);
        animationEnded = theta >= 9.0*PI/16.0;
        if(animationEnded){
            //Exclamation dot
            canvas -> fillCircle(width/2, height/2 + 4.0*plottingRadius/7.0,
                            outerPincelStroke, animationColor);
        }

    }else if(animationID == ANIM_PLAY_TRIANGLE){
        animationEnded = drawNGon(3, 0, 0);
    }else if(animationID == ANIM_PAUSE){
        double theta = getTickTime()/1000.0*TWO_PI* plottingSpeed;
        double k = 10, n = 4, a = 0.5, b = 2;
        double r = k*a*b/pow(pow(b*cos(theta), n) + pow(a*sin(theta), n), 1/n);
        canvas->fillCircle(7.0*width/18.0 + r*cos(theta), height/2 + r*sin(theta),
                    outerPincelStroke, animationColor);
        canvas->fillCircle(11.0*width/18.0 + r*cos(theta), height/2 + r*sin(theta),
                    outerPincelStroke, animationColor);
        animationEnded = theta >= TWO_PI;

    }else if((animationID&0x40) == ANIM_POLYGON){
        uint8_t shape = animationID&0x0F;
        double rotAngle = 0;
        double sizeTweak = 0;
        if(shape == 3){
            rotAngle = -0.524;
            sizeTweak = 10;
        }else if(shape == 4) rotAngle = 0.785;
        else if(shape == 5) rotAngle = 0.315;
        else if(shape == 7) rotAngle = -0.224; 
        animationEnded = drawNGon(shape, rotAngle, sizeTweak);
        
    }else if(animationID == ANIM_TEXT){
        canvas->setTextColor(animationColor);
        canvas->setTextDatum(MC_DATUM);
        canvas->drawString(animationText, width/2, 0.5*(plottingRadius+3.0*height/2.0));
        animationEnded = true;
    }

    if(animationEnded){
        if(hasMultipleAnimation){
            currentAnimationIndex++;
            if(currentAnimationIndex != animationQueue.size()){
                animationID = animationQueue[currentAnimationIndex];
                setPalette();
                startAnimation();
                return;
            }
            //else: reached the end of the animation queue
        }
        //canvas->setRotation(1);
        endAnimation();
        animationQueue.clear();
        animationQueuePalette.clear();
        MenuManager::wakeUpDrawTask();
    }else animationRedraw();
}

// Idea "leased" from https://www.geogebra.org/m/cXXGKUQk
// Since TFT screen's coords are Y flipped, take that in mind during the dessign!
bool DisplayOverlay::drawNGon(uint8_t sides, double rotAngle, double sizeTweak){
    double theta = getTickTime()/1000.0*TWO_PI * plottingSpeed;
    double y = TWO_PI/sides;
    double theta_rot = theta + rotAngle;

    if(theta < TWO_PI){
        double mod = theta_rot - y*floor(theta_rot/y);
        double r = cos(PI/sides)/cos(mod - PI/sides)*(plottingRadius+sizeTweak);
        canvas->fillCircle(width/2 + r*cos(theta), height/2 + r*sin(theta),
                        outerPincelStroke, animationColor);
    }
    double mod = theta_rot - y*floor(theta_rot/y);
    double r = cos(PI/sides)/cos(mod - PI/sides)*(plottingRadius+sizeTweak) * theta_rot / (4.0*PI);
    canvas->fillCircle(width/2 + r*cos(theta), height/2 + r*sin(theta),
                        innerPincelStroke, animationColor);

    return theta >= 3.75*PI;
}
#else
void DisplayOverlay::draw(){
    bool animationEnded = false;
    if(animationID == ANIM_WAIT){
        long t = getTickTime();
        animationEnded = t > waitTime;
    }else if(animationID == ANIM_SWEEP_IN_LEFT || animationID == ANIM_SWEEP_OUT_LEFT ||
             animationID == ANIM_SWEEP_IN_RIGHT || animationID == ANIM_CIRCLE){
        canvas->fillSprite(animationColor);
        animationEnded = true;
    }else if(animationID == ANIM_CIRCUMFERENCE){
        canvas->fillCircle(width/2, height/2, plottingRadius, animationColor);
        animationEnded = true;
    }else if(animationID == ANIM_EXCLAMATION){
        for(double theta = 7.0*PI/16.0; theta <= 9.0*PI/16.0; theta += 0.2*PI/16.0){
            double r = -7.0*plottingRadius*cos(8.0*theta)/8.0;
            double c1 = height/2 + 7.0*plottingRadius/32.0;
            //This is part of a rose of 8 pethals. Is the bar of the exclamation.
            canvas->fillCircle(width/2 + r*cos(theta), c1 + r*sin(theta),
                                innerPincelStroke, animationColor);
        }
        //Exclamation dot
        canvas -> fillCircle(width/2, height/2 + 4.0*plottingRadius/7.0,
                        outerPincelStroke, animationColor);
        animationEnded = true;
    }else if(animationID == ANIM_PLAY_TRIANGLE){
        animationEnded = drawNGon(3, 0, 0);
    }else if(animationID == ANIM_PAUSE){
        const double k = 10, n = 4, a = 0.5, b = 2;
        for(double theta = 0; theta <= TWO_PI; theta+=0.1){
            double r = k*a*b/pow(pow(b*cos(theta), n) + pow(a*sin(theta), n), 1/n);
            canvas->fillCircle(7.0*width/18.0 + r*cos(theta), height/2 + r*sin(theta),
                        outerPincelStroke, animationColor);
            canvas->fillCircle(11.0*width/18.0 + r*cos(theta), height/2 + r*sin(theta),
                        outerPincelStroke, animationColor);
        }
        animationEnded = true;
    }else if((animationID&0x40) == ANIM_POLYGON){
        uint8_t shape = animationID&0x0F;
        double rotAngle = 0;
        double sizeTweak = 0;
        if(shape == 3){
            rotAngle = -0.524;
            sizeTweak = 10;
        }else if(shape == 4) rotAngle = 0.785;
        else if(shape == 5) rotAngle = 0.315;
        else if(shape == 7) rotAngle = -0.224; 
        animationEnded = drawNGon(shape, rotAngle, sizeTweak);
    }else if(animationID == ANIM_TEXT){
        canvas->setTextColor(animationColor);
        canvas->setTextDatum(MC_DATUM);
        canvas->drawString(animationText, width/2, 0.5*(plottingRadius+3.0*height/2.0));
        animationEnded = true;
    }

    animationRedraw();

    if(animationEnded){
        if(hasMultipleAnimation){
            currentAnimationIndex++;
            if(currentAnimationIndex != animationQueue.size()){
                animationID = animationQueue[currentAnimationIndex];
                setPalette();
                startAnimation();
                return;
            }
            //else: reached the end of the animation queue
        }
        //canvas->setRotation(1);
        endAnimation();
        animationQueue.clear();
        animationQueuePalette.clear();
        //MenuManager::wakeUpDrawTask();
    }
}

bool DisplayOverlay::drawNGon(uint8_t sides, double rotAngle, double sizeTweak){
    for(double theta = 0; theta <= 3.75*PI; theta += 0.1){
        double y = TWO_PI/sides;
        double theta_rot = theta + rotAngle;
        double mod = theta_rot - y*floor(theta_rot/y);
        
        if(theta < TWO_PI){
            double r = cos(PI/sides)/cos(mod - PI/sides)*(plottingRadius+sizeTweak);
            canvas->fillCircle(width/2 + r*cos(theta), height/2 + r*sin(theta),
                            outerPincelStroke, animationColor);
        }
        double r = cos(PI/sides)/cos(mod - PI/sides)*(plottingRadius+sizeTweak) * theta_rot / (4.0*PI);
        canvas->fillCircle(width/2 + r*cos(theta), height/2 + r*sin(theta),
                            innerPincelStroke, animationColor);
    }
    return true;
}
#endif

void DisplayOverlay::drawAnimation(uint8_t animation){
    hasMultipleAnimation = false;
    animationID = animation;
    setPalette();
    startAnimation();
}

void DisplayOverlay::drawMultipleAnimation(std::vector<uint8_t> q){
    hasMultipleAnimation = true;
    animationQueue = q;
    animationID = animationQueue[0];
    currentAnimationIndex = 0;
    animationQueueSize = q.size();
    setPalette();
    startAnimation();
}

void DisplayOverlay::drawMultipleAnimation(std::vector<uint8_t> q, std::vector<uint16_t> colors){
    if(q.size() != colors.size()) Serial.println("Animation count and color pallete aren't the same size");
    hasMultipleAnimation = true;
    animationQueue = q;
    animationQueuePalette = colors;
    animationID = animationQueue[0];
    currentAnimationIndex = 0;
    animationQueueSize = q.size();
    setPalette();
    startAnimation();
}

void DisplayOverlay::setAnimationText(String str){
    animationText = str;
}

void DisplayOverlay::setPalette(){
    if(animationQueuePalette.size() > 0){
        if(currentAnimationIndex >= animationQueuePalette.size()) animationColor = TFT_PINK;
        else animationColor = animationQueuePalette[currentAnimationIndex];
    }else{ //DEFAULT Values
        if(animationID == ANIM_SWEEP_OUT_LEFT) animationColor = TFT_BLACK;
        else animationColor = TFT_YELLOW;
    } 
}