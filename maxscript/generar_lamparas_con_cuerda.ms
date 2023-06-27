clearListener()

function generateRope a b = (
	
	--para que sea curva la cuerda le ponemos dos puntos por los que debe pasar
	local ab = distance a b
	local down = -[0,0,1] *ab*0.1
	local a1 = a + (b - a)*0.3
	local b1 = b + (a - b)*0.3
	a1 = a1 - [0,0,1]*ab*0.2
	b1 = b1 - [0,0,1]*ab*0.2
	
	
	--Create the shape
	local sp = splineShape()
	addnewSpline sp --esto lo buscamos en el help de 3dsmax
	addKnot sp 1 #bezierCorner #curve a a1 a1
	addKnot sp 1 #bezierCorner #curve b b1 b1
	updateShape sp
	sp.render_displayRenderMesh = true
	
	local delta_t=0.1 --incremento --modificar si queremos que esten mas juntas o separadas
	local t = 0.1
	while t<= 0.9 do (
		local new_point = interpCurve3D sp 1 t
		--local b =Box pos:new_point scale:[0.1,0.1,0.1]
		
		new_point = new_point + down*0.5
		
		local b = instance $Cylinder001 pos:new_point --sustituir el cylinder por el nombre de la lampara
		--b.radius = ab*0.05
		--b.segs = 1
		--b.smooth = off
		
		
		t = t + delta_t
	)
	
)

delete $Box*
delete $Geo*
delete $Shape*

generateRope $Dummy001.position $Dummy002.position --sustiruir por los dumies que sean 