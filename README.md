# Curve_Editor
Curve editor that implements polylines, Bezier curves, and Lagrange curves and some manipulations of the curves

Program draws each curve that the user makes by clicking on the screen where contorl points should be. Holding p draws a polyline (drawn in red), b draws a Bezier curve (drawn in purple), and l draws a Lagrane curve (drawn in green). A curve with one control point is immediately deleted, so hold down the key for long enough to draw at least two control points. When a curve is drawn, it becomes the selected curve and the color becomes blue, and the control points become visible. Pressing space bar cycles through selecting each curve in the order they were drawn.

Holding down different keys and then clicking the mouse also affects the scene. The keys p, b, and l have already been discussed. Holding a and pressing the mouse adds a control point to the selected curve and then redraws the curve. Holding d and clicking deletes a control point from the curve, if there is a contol point near the click. Holding e and clicking deletes the whole selected curve. If no key is pressed, then the program looks to see if a curve is near the location of the click and then selects that curve.

All methods and classes implemented by me. A float2.h file is included in order to run the program
