#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// Needed on MsWindows
#include <windows.h>
#endif // Win32 platform

#include <vector>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#include "float2.h"
//--------------------------------------------------------
// Display callback that is responsible for updating the screen
//--------------------------------------------------------


//Globals used later in the program to see which keys are pressed
bool ppressed = false;
bool lpressed = false;
bool bpressed = false;
bool apressed = false;
bool dpressed = false;
bool epressed = false;
bool nothingPressed = true;


//Super class for the rest of the curves with simple methods
class Curve
{
protected:
    std::vector<float2> controlPoints;
public:
    bool selected = false;
    virtual float2 getPoint(float t)=0;
    virtual void addControlPoint(float x, float y){}
    virtual void draw(){}
    
    void select()
    {
        selected = true;
    }
    
    void deselect()
    {
        selected = false;
    }
    int controlPointsSize()
    {
        return (int)controlPoints.size();
    }
    float2 getControlPoint(int i)
    {
        float2 temp;
        temp.x = controlPoints[i].x;
        temp.y = controlPoints[i].y;
        return temp;
    }
    virtual void removeControlPoint(int x)
    {
        controlPoints.erase(controlPoints.begin()+x);
    }
};


//Polyline with minimal interpolation. Just draws a line between each control point
class PolyLine : public Curve
{
public:
    
    //not used to draw the curve, but I use this to parameterize the polyline for clicking and selecting curves
    float2 getPoint (float t)
    {
        float2 finalPoint(0.0,0.0);
        float segments = controlPointsSize();
        segments = segments-1;
        double segmentation = 1/segments;
        float whichControlPointtoStartFrom = 0;
        
        for (int i = 1; i<controlPointsSize(); i++)
        {
            if (t>segmentation*i)
                whichControlPointtoStartFrom++;
        }
        
        float2 startPoint = getControlPoint(whichControlPointtoStartFrom);
        float startx = startPoint.x;
        float starty = startPoint.y;
        float2 endPoint = controlPoints[whichControlPointtoStartFrom+1];
            
        float slope = (endPoint.y-startPoint.y)/(endPoint.x-startPoint.x);
        float changeiny = (endPoint.y-startPoint.y);
        float changeinx = (endPoint.x-startPoint.x);
        //doesn't work if the line is vertical. I handle the error here
        if (slope < std::numeric_limits<double>::lowest() || slope>std::numeric_limits<double>::max())
        {
            slope = 0;
        }
        
        t = t-(segmentation*whichControlPointtoStartFrom);
        
        float finalx = startx;
        float finaly = starty;
        
        for (float i = 0; i<t; i+=0.1)
        {
            finalx = finalx+changeinx*t;
            finaly = finaly+changeiny*t;
        }
        finalPoint.x = finalx;
        finalPoint.y = finaly;
        return finalPoint;
    }
    
    float getSize()
    {
        return controlPoints.size();
    }
    
    void addControlPoint(float x, float y)
    {
        float2 temp;
        temp.x = x;
        temp.y = y;
        controlPoints.push_back(temp);
    }
    
    void drawControlPoints(){
        glBegin(GL_POINTS);
        for (int i = 0; i<controlPoints.size(); i++)
        {
            glVertex2d(controlPoints[i].x, controlPoints[i].y);
        }
        glEnd();
    }
    
    void draw()
    {
        if (selected)
        {
            glLineWidth(4);
            glColor3f(0.0, 0.0, 1.0);
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i<controlPoints.size(); i++)
            {
                glVertex2d(controlPoints[i].x, controlPoints[i].y);
            }
            glEnd();
            glColor3f(0.0, 0.0, 0.0);
            drawControlPoints();
            glLineWidth(2);
        }
        else
        {
            glColor3f(1.0, 0.0, 0.0);
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i<controlPoints.size(); i++)
            {
                glVertex2d(controlPoints[i].x, controlPoints[i].y);
            }
            glEnd();
        }

    }
    
};



//Superclass for Bezier and Lagrange curves
class Freeform : public Curve
{
public:
    virtual float2 getPoint(float t) = 0;
    virtual void draw()
    {
    }
    virtual void addControlPoint(float x, float y)
    {
        float2 p;
        p.x = x;
        p.y = y;
        controlPoints.push_back(p);
    }
    
    void drawControlPoints(){
        glBegin(GL_POINTS);
        for (int i = 0; i<controlPoints.size(); i++)
        {
            glVertex2d(controlPoints[i].x, controlPoints[i].y);
        }
        glEnd();
    }
    
    
    
    
};

class BezierCurve : public Freeform
{
    //The next two methods are the implementation of Bezier interpolation
    static double bernstein(int i, int n, double t)
    {
        if (n == 1){
            if (i == 0) return 1-t;
            if (i == 1) return t;
            return 0;
        }
        if (i<0 || i>n) return 0;
        return (1-t) * bernstein(i, n-1, t) + t * bernstein(i-1, n-1, t);
    }
    
    float2 getPoint(float t)
    {
        float2 r(0.0, 0.0);
        for (int i = 0; i<controlPoints.size(); i++)
        {
            float weight;
            weight = bernstein(i,(int)controlPoints.size()-1, t);
            r += controlPoints[i]*weight;
        }
        return r;
    }
    
    //Draw method divides the curve into 100 subsections and draws lines between points that are determined by Bezier interpolation
    void draw()
    {
        if (selected)
        {
            glLineWidth(4);
            glColor3f(0.0, 0.0, 1.0);
            glBegin(GL_LINES);
            for (float t = 0; t<0.99; t+=0.01)
            {
                glVertex2d(getPoint(t).x, getPoint(t).y);
                glVertex2d(getPoint(t+0.01).x, getPoint(t+0.01).y);
            }
            glEnd();
            glColor3f(0.0, 0.0, 0.0);
            drawControlPoints();
            glLineWidth(2);
        }
        else
        {
            glColor3f(0.5, 0.7, 0.3);
            glBegin(GL_LINES);
            for (float t = 0; t<0.99; t+=0.01)
            {
                glVertex2d(getPoint(t).x, getPoint(t).y);
                glVertex2d(getPoint(t+0.01).x, getPoint(t+0.01).y);
            }
            glEnd();
        }
    }
};

class LagrangeCurve : public Freeform
{
protected:
    std::vector<float> knots;
    
public:
    //The next two methods are the implementation of Lagrange interpolation
    void addControlPoint(float x, float y)
    {
        float2 p;
        p.x = x;
        p.y = y;
        controlPoints.push_back(p);
        knots.clear();
        if (controlPoints.size() > 1) {
            for(int i=0; i<controlPoints.size(); i++)
            {
                float ti = (float)i/(controlPoints.size()-1);
                knots.push_back(ti);
            }
        }
    }
    
    float2 getPoint(float t)
    {
        float2 r(0.0, 0.0);
        for (int i = 0; i<controlPoints.size(); i++)
        {
            float weight=1;
            for (int j = 0; j<controlPoints.size(); j++)
            {
                if (j!=i)
                    weight = weight*(t-knots[j])/(knots[i]-knots[j]);
            }
            r += controlPoints[i]*weight;
        }
        return r;
    }
    
    void removeControlPoint(int x)
    {
        controlPoints.erase(controlPoints.begin()+x);
        knots.clear();
        if (controlPoints.size() > 1) {
            for(int i=0; i<controlPoints.size(); i++)
            {
                float ti = (float)i/(controlPoints.size()-1);
                knots.push_back(ti);
            }
        }
    }
    
    //Draw works the same way as the Bezier curves, where the curve is divided into 100 subsections and lines are drawn between each subsection
    void draw()
    {
        if (selected)
        {
            glLineWidth(4);
            glColor3f(0.0, 0.0, 1.0);
            glBegin(GL_LINES);
            for (float t = 0; t<0.99; t+=0.01)
            {
                glVertex2d(getPoint(t).x, getPoint(t).y);
                glVertex2d(getPoint(t+0.01).x, getPoint(t+0.01).y);
            }
            glEnd();
            glColor3f(0.0, 0.0, 0.0);
            drawControlPoints();
            glLineWidth(2);
        }

        else
        {
            glColor3f(0.7, 0.0, 0.9);
            glBegin(GL_LINES);
            for (float t = 0; t<0.99; t+=0.01)
            {
                glVertex2d(getPoint(t).x, getPoint(t).y);
                glVertex2d(getPoint(t+0.01).x, getPoint(t+0.01).y);
            }
            glEnd();
        }
    }
};

float distance(float x1, float y1, float x2, float y2)
{
    return sqrt(((x2-x1)*(x2-x1))+((y2-y1)*(y2-y1)));
}

//The class that keeps track of the curves that are drawn and can perform operations on each of them
class Scene
{
    std::vector<Curve*> curves;
public:
    
    int selectedIndex = 0;
    
    void addObject(Curve* curve) {
        curves.push_back(curve);
        selectLast();
    }
    ~Scene() {
        for(unsigned int i=0; i<curves.size(); i++)
            delete curves.at(i);
    }
    void draw() {
        for(unsigned int i=0; i<curves.size(); i++)
            curves.at(i)->draw();
        
    }
    void deleteBadCurves()
    {
        for(unsigned int i=0; i<curves.size(); i++)
        {
            if (curves[i]->controlPointsSize()<2)
                curves.erase(curves.begin()+i);
        }
    }
    void deleteSelcted()
    {
        if (selectedIndex<curves.size())
        {
            for(unsigned int i=0; i<curves.size(); i++)
            {
                if (i==selectedIndex)
                    curves.erase(curves.begin()+i);
            }
            }
    }
    Curve getCurve(int i)
    {
        return *curves[i];
    }
    
    int getSize()
    {
        return (int)curves.size();
    }
    
    void selectNext()
    {
        if (selectedIndex<curves.size())
            selectedIndex++;
        else
            selectedIndex = 0;
        for (int i = 0; i<curves.size(); i++)
        {
            curves[i]->deselect();
        }
        if (selectedIndex<curves.size())
            curves[selectedIndex]->select();
    }

    void selectLast()
    {
        for (int i = 0; i<curves.size(); i++)
        {
            curves[i]->deselect();
        }
        curves[curves.size()-1]->select();
        selectedIndex = (int)curves.size()-1;
    }
    
    void selectCurve(int i)
    {
        for (int i = 0; i<curves.size(); i++)
        {
            curves[i]->deselect();
        }
        if (selectedIndex<curves.size())
            curves[i]->select();
        selectedIndex = i;
    }
    
    void addControlPointToIndex(float x, float y)
    {
        curves[selectedIndex]->addControlPoint(x,y);
    }
    
    void addControlPointToLast(float x, float y)
    {
        curves[curves.size()-1]->addControlPoint(x,y);
    }
    
    void removeControlPoint(int p)
    {
        curves[selectedIndex]->removeControlPoint(p);
    }
    
    float2 getPoint(int i, float t)
    {
        return curves[i]->getPoint(t);
    }
};

Scene scene;

void onDisplay( ) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPointSize(6.0);
    
    if (nothingPressed)
        scene.deleteBadCurves();
    scene.draw();

    glutSwapBuffers();
}

void onIdle(){
    glutPostRedisplay();
}

//used to see if a new curve needs to be created or if control points need to be added to an existing curve
int counter = 0;

//The next two methods that reads the keys being pressed and changes the global booleans according to the keys being pressed
void onKeyboardUp(unsigned char key, int x, int y)
{
    if (key == 'p')
    {
        counter = 0;
        ppressed = false;
        nothingPressed = true;
    }
    
    if (key == 'b')
    {
        counter = 0;
        bpressed = false;
        nothingPressed = true;
    }
    
    if (key == 'l')
    {
        counter = 0;
        lpressed = false;
        nothingPressed = true;
    }
    if (key == 'a')
    {
        apressed = false;
        nothingPressed = true;
    }
    if (key == 'd')
    {
        dpressed = false;
        nothingPressed = true;
        
    }
    if (key == 'e')
    {
        epressed = false;
        nothingPressed = true;
        
    }
}

void onKeyboard(unsigned char key, int x, int y)
{
   if (key == 'p')
   {
       if (counter == 0)
           counter = 1;
       ppressed = true;
       
       nothingPressed = false;
   }
    
    if (key == 'b')
    {
        if (counter == 0)
            counter = 1;
        bpressed = true;
        nothingPressed = false;
    }
    
    if (key == 'l')
    {
        if (counter == 0)
            counter = 1;
        lpressed = true;
        nothingPressed = false;
    }
    
    
    if (key == 'a')
    {
        apressed = true;
        nothingPressed = false;
    }
    if (key == 'd')
    {
        dpressed = true;
        nothingPressed = false;
    }
    if (key == 'e')
    {
        epressed = true;
        nothingPressed = false;
    }
    
    //Pressing the spacebar cycles through the curves, selecting them in the order they were added to the Scene
    if ((int)key == 32)//spacebar
    {
        scene.selectNext();
    }
}

//Depending on which key is pressed, clicking the mouse can have different effects on the Scene. The if statements handle what happens when a key is pressed and the mouse is clicked
void onMouse(int button, int state, int x, int y) {
    int viewportRect[4];
    glGetIntegerv(GL_VIEWPORT, viewportRect);
    PolyLine *poly;
    BezierCurve *bez;
    LagrangeCurve *lang;
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        //If p is pressed, add a polyline to Scene and keep adding control points for every click until p is released. The control point adding is taken care of in the else if (ppressed && counter > 1) below
        if (ppressed && counter == 1)
        {
            counter++;
            poly = new PolyLine();
            poly->addControlPoint(x * 2.0 / viewportRect[2] - 1.0,-y * 2.0 / viewportRect[3] + 1.0);
            scene.addObject(poly);
            scene.draw();
        }
        //If b is pressed, add a Bezier Curve to Scene and keep adding control points for every click until b is released. The control point adding is taken care of in the else if (bpressed && counter > 1) below
        else if (bpressed && counter == 1)
        {
            counter++;
            bez = new BezierCurve();
            bez->addControlPoint(x * 2.0 / viewportRect[2] - 1.0,-y * 2.0 / viewportRect[3] + 1.0);
            scene.addObject(bez);
            scene.draw();
        }
        //If l is pressed, add a Lagrange curve to Scene and keep adding control points for every click until l is released. The control point adding is taken care of in the else if (lpressed && counter > 1) below
        else if (lpressed && counter == 1)
        {
            counter++;
            lang = new LagrangeCurve();
            lang->addControlPoint(x * 2.0 / viewportRect[2] - 1.0,-y * 2.0 / viewportRect[3] + 1.0);
            scene.addObject(lang);
            scene.draw();
        }
        else if (ppressed && counter > 1)
        {
            scene.addControlPointToLast(x * 2.0 / viewportRect[2] - 1.0,-y * 2.0 / viewportRect[3] + 1.0);
            scene.draw();
        }
        else if (bpressed && counter > 1)
        {
            scene.addControlPointToLast(x * 2.0 / viewportRect[2] - 1.0,-y * 2.0 / viewportRect[3] + 1.0);
            scene.draw();
        }
        else if (lpressed && counter > 1)
        {
            scene.addControlPointToLast(x * 2.0 / viewportRect[2] - 1.0,-y * 2.0 / viewportRect[3] + 1.0);
            scene.draw();
        }
        //If a is pressed, add a control point to the selected curve
        else if (apressed)
        {
            if (scene.selectedIndex!=scene.getSize())
            {
                scene.addControlPointToIndex(x * 2.0 / viewportRect[2] - 1.0, -y * 2.0 / viewportRect[3] + 1);
                scene.draw();
            }
        }
        //If d is pressed and the click is near a control point of the selected curve, delete that control point
        else if (dpressed)
        {
            if (scene.selectedIndex!=scene.getSize())
            {
                float xloc = x * 2.0 / viewportRect[2] - 1.0;
                float yloc = -y * 2.0 / viewportRect[3] + 1;
                for (int i = 0; i<scene.getCurve(scene.selectedIndex).controlPointsSize(); i++)
                {
                    float2 controlPoint = scene.getCurve(scene.selectedIndex).getControlPoint(i);
                    if (distance(controlPoint.x, controlPoint.y, xloc, yloc)<0.05)
                    {
                        scene.removeControlPoint(i);
                    }
                }
            }
            
            
        }
        //If e is pressed, then delete the selected curve
        else if (epressed)
        {
            scene.deleteSelcted();
        }
        
        //If nothing is pressed, look near where the click was, and if there is a curve nearby, select that curve
        else if (nothingPressed)
        {
            float xloc = x * 2.0 / viewportRect[2] - 1.0;
            float yloc = -y * 2.0 / viewportRect[3] + 1;
            
            for (int i = 0; i<scene.getSize(); i++)
            {
                for (float t = 0; t<0.99; t+=0.01)
                {
                    float2 temp;
                    temp = scene.getPoint(i,t);
                    if (distance(temp.x, temp.y, xloc, yloc)<0.1)
                    {
                        scene.selectCurve(i);
                    }
                }
            }
        }
    }
    glutPostRedisplay();
}


//--------------------------------------------------------
// The entry point of the application
//--------------------------------------------------------
int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Curves");
    glutKeyboardFunc(onKeyboard);
    glutKeyboardUpFunc(onKeyboardUp);
    glutDisplayFunc(onDisplay);
    glutIdleFunc(onIdle);
    glutMouseFunc(onMouse);
    glutMainLoop();
    return 0;
}
