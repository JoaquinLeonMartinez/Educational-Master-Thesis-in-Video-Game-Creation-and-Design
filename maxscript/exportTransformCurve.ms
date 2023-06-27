clearListener()


-- ----------------------------------------------------------
-- ----------------------------------------------------------
-- ----------------------------------------------------------
struct TCurveTransformExporter (
	
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
	
	fn exportTransformCurve obj res= (
		if not isCurve obj then 
			return undefined
		if not isValidName obj.name then
			return undefined
		--C:/Users/victo/mcv_sm/bin/data/prefabs/cameras
		local full_path = "C:/Users/victo/mcv_sm/bin/data/curves/"+obj.name+".trans_curve"
		format "Exporting to %\n" full_path 
		fs.begin full_path
		fs.beginArray()
		
		local steps=res - 1
		local stepLength = 1.0 / steps
		
		local step0 = 0.0
		local firstPoint = (interpCurve3D obj 1 step0)
		firstPoint = firstPoint-- - obj.position
		local step1 = stepLength
		local secondPoint = (interpCurve3D obj 1 step1)
		secondPoint = secondPoint-- - obj.position
		local deltaEntrance = firstPoint - secondPoint
		local entrance = firstPoint + deltaEntrance
		
		local max2mcv = rotateXMatrix -90
		
		entrance = [entrance.x, entrance.z, entrance.y]
		local mcvEntrance = entrance * max2mcv
		fs.beginObj()
		fs.writeKey "pos"
		fs.writeVec3 mcvEntrance
		fs.writeComma()
		local lookat_ = getUserProp obj "lookat"
		fs.writeKeyValue "lookat" lookat_
		fs.endObj()
		fs.writeComma()
		for i = 0 to steps do 
		(
			fs.beginObj()
			fs.writeKey "pos"
			local actualStep = stepLength * i
			format "actualstep used % \n" actualStep
			local res = interpCurve3D obj 1 actualStep
			res = res-- - obj.position
			local engineCurvePointPos = res * max2mcv
			--local engineCurvePointPos = [res.x, res.z, res.y]
			fs.writeVec3 engineCurvePointPos
			fs.writeComma()
			--lookAt de camera
			local lookat_ = getUserProp obj "lookat"
			fs.writeKeyValue "lookat" lookat_
			fs.endObj()
			--fs.writeKeyValue "lookat" lookat_
			fs.writeComma()
		)
		
		local stepAlmostEnd = (1.0 - stepLength)
		local almostEndPoint = (interpCurve3D obj 1 stepAlmostEnd)
		almostEndPoint = almostEndPoint-- - obj.position
		local stepEnd = 1.0
		local endPoint = (interpCurve3D obj 1 stepEnd)
		endPoint = endPoint - obj.position
		local deltaEnd = endPoint-- - almostEndPoint
		local end = endPoint + deltaEnd
		
		format "end point % % % %\n" endPoint.x endPoint.z endPoint.y stepEnd
		format "almost end point % % % %\n" almostEndPoint.x almostEndPoint.z almostEndPoint.y stepAlmostEnd
		
		end = [end.x, end.z, end.y]
		
		fs.beginObj()
		local engineEnd = end * max2mcv
		fs.writeKey "pos"
		fs.WriteVec3 engineEnd
		fs.writeComma()
		local lookat_ = getUserProp obj "lookat"
		fs.writeKeyValue "lookat" lookat_
		fs.endObj()
		fs.endArray()
		fs.end()
	),
	
	-- Will return an array of all the materials names used by obj and exported by us
	fn exportAll = (
		for obj in $* do (
			if isCurve obj then
			exportTransformCurve obj
		)
	)

	
)
