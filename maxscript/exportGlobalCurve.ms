clearListener()


-- ----------------------------------------------------------
-- ----------------------------------------------------------
-- ----------------------------------------------------------
struct TCurveGlobalExporter (
	
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
	
	fn exportGlobalCurve obj res= (
		if not isCurve obj then 
			return undefined
		if not isValidName obj.name then
			return undefined
		--C:/Users/victo/mcv_sm/bin/data/prefabs/cameras
		local full_path = "C:/Users/victo/mcv_sm/bin/data/curves/"+obj.name+".curve"
		format "Exporting to %\n" full_path 
		fs.begin full_path
		fs.beginArray()
		
		local steps=res - 1
	
		local max2mcv = rotateXMatrix -90
		local pointCurvei = (interpCurve3D obj 1 0)
		local engineCurvePointPosi = pointCurvei * max2mcv
		
		fs.writeVec3 engineCurvePointPosi
		fs.writeComma()
		for i = 0 to steps do 
		(
			
			
		)
	
		local pointCurvef = (interpCurve3D obj 1 2)
		local engineCurvePointPosf = pointCurvef * max2mcv
			
		fs.writeVec3 engineCurvePointPosf
		
		fs.endArray()
		fs.end()
	),
	
	-- Will return an array of all the materials names used by obj and exported by us
	fn exportAll = (
		for obj in $* do (
			if isCurve obj then
			exportGlobalCurve obj res
		)
	)

	
)
