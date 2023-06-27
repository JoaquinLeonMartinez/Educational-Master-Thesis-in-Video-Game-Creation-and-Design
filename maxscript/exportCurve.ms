clearListener()


-- ----------------------------------------------------------
-- ----------------------------------------------------------
-- ----------------------------------------------------------
struct TCurveExporter (
	
	fs = TJsonFormatter(), 
	project_path,                 -- Asigned in the ctor 
	base_path,                 -- Asigned in the ctor 	
	
	fn isValidName aname = (
		-- Add no Ñ, accents, etc.
		return findString aname " " == undefined 
	),
	
	fn isCurve obj = (
		return obj != undefined and  classOf obj == line
	),
	
	fn exportCurve obj res= (
		if not isCurve obj then 
			return undefined
		if not isValidName obj.name then
			return undefined
		--C:/Users/victo/mcv_supermarket/MCV_Supermarket/bin/data/
		--C:/Users/mcv17_alumne/Desktop/mcv_sm/bin/data/curves/ --C:\Users\joaki\Desktop\VisualStudioProjects\Proyecto_43\mcv_sm\bin\data\
		local full_path = "C:/Users/victo/mcv_supermarket/MCV_Supermarket/bin/data/curves/"+obj.name+".curve"
		format "Exporting to %\n" full_path 
		fs.begin full_path
		fs.beginArray()
		
		local steps=res - 1
		local stepLength = 1.0 / steps
		
		local step0 = 0.0
		local firstPoint = (interpCurve3D obj 1 step0)
		
		local step1 = stepLength
		local secondPoint = (interpCurve3D obj 1 step1)
		
		local deltaEntrance = firstPoint - secondPoint
		local entrance = firstPoint + deltaEntrance
		local max2mcv = rotateXMatrix -90
		
		entrance = [entrance.x, entrance.y, entrance.z]
		local mcvEntrance = entrance * max2mcv
		fs.writeVec3 mcvEntrance
		fs.writeComma()
		for i = 0 to steps do 
		(
			local actualStep = stepLength * i
			format "actualstep used % \n" actualStep
			local res = interpCurve3D obj 1 actualStep
			res = res-- - obj.position
			local engineCurvePointPos = res * max2mcv
			--local engineCurvePointPos = [res.x, res.z, res.y]
			fs.writeVec3 engineCurvePointPos
			fs.writeComma()
		)
		
		local stepAlmostEnd = (1.0 - stepLength)
		local almostEndPoint = (interpCurve3D obj 1 stepAlmostEnd)
				
		local stepEnd = 1.0
		local endPoint = (interpCurve3D obj 1 stepEnd)
		
		local deltaEnd = endPoint - almostEndPoint
		local end = endPoint + deltaEnd
		
		format "end point % % % \n" endPoint.x endPoint.y endPoint.z
		format "almost end point % % % \n" almostEndPoint.x almostEndPoint.y almostEndPoint.z
		format "deltaEnd % % % " deltaEnd.x deltaEnd.y deltaEnd.z

		
		
		end = [end.x, end.y, end.z]
		local engineEnd = end * max2mcv
		fs.WriteVec3 engineEnd
		
		fs.endArray()
		fs.end()
	),
	
	-- Will return an array of all the materials names used by obj and exported by us
	fn exportAll = (
		for obj in $* do (
			if isCurve obj then
			exportCurve obj
		)
	)

	
)
