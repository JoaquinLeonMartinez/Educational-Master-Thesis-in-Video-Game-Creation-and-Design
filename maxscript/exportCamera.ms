clearListener()

struct TCameraExporter(
	
	fs = TJsonFormatter(), 
	project_path,                 
	base_path,

	fn isValidName aname = (
		return findString aname " " == undefined 
	),
	
	fn isCamera obj = (
		return obj != undefined and  (classOf obj == targetCamera) 
	),
	
	fn exportCompName obj = (
			
		fs.writeKeyValue "name" obj.name
	
		
	),
	
	fn exportTransform obj = (
		fs.writeComma()
		fs.writeKey "transform" 
		
		local max2mcv = rotateXMatrix -90
		local mcv2max = rotateXMatrix 90
		local mcv_position = obj.position * max2mcv
		
		-- From mcv, we will go to max, apply the max transform and go back to mcv coord system
		local mcv_transform = mcv2max * obj.transform * max2mcv

		-- Take just the rotation as quaterion
		local mcv_quat = mcv_transform.rotationPart as quat
		
		fs.beginObj()
		fs.writeKeyValue "pos" mcv_position
		fs.writeComma()
		fs.writeKeyValue "rotation" mcv_quat			
		fs.writeComma()
		local mcv_look_at = obj.Target.pos * max2mcv
		fs.writeKeyValue "lookat" mcv_look_at
		fs.endObj()
	),
	
	fn exportCamera obj = (
		
		local znear = obj.nearclip
		local zfar = obj.farclip
		local fov = obj.fov
		--local pos =  obj.position
		--local rot = obj.transform.rotation
		local targetDistance = obj.targetDistance
		fs.writeComma()
		fs.writeKey "camera"
		fs.beginObj()
		fs.writeKeyValue "near" znear
		fs.writeComma()
		fs.writeKeyValue "far" zfar
		fs.writeComma()
		fs.writeKeyValue "fov" fov
		fs.writeComma()
		fs.writeKeyValue "distanceTarget" targetDistance
		fs.endObj()
		
	),
	
	fn findCameraTarget=(
		
		for obj in $* do (
			if isCamera  obj  then (
				
				return obj	
			)
		)
		return undefined
	),

	fn createAllCamera=(
		local obj = findCameraTarget()
		if not isCamera obj then
			return undefined
		if not isValidName obj.name then
			return undefined
		--C:\Users\victo\mcv_sm\bin\data\cameras
		
		local full_path = "C:/Users/victo/mcv_sm/bin/data/prefabs/cameras/"+obj.name+".json"
		
		format "Exporting to %\n" full_path
		fs.begin full_path
		fs.beginArray()
		
			fs.beginObj()
			format "% is a camera target\n" obj.name
			fs.writeKey "entity"
				fs.beginObj()
				exportCompName obj
				exportTransform obj
				exportCamera obj
				fs.endObj()
			fs.endObj()
		fs.endArray()
		fs.end()
	)
	
)	
--------------------------------------------------------------------------------------------------
	
	--solo exportar una camera
	--fn exportCamera obj =(
		--if not isCamera obj then 
			--return undefined
		--if not isValidName obj.name then
			--return undefined
		
		--local znear = obj.nearclip
		--local zfar = obj.farclip
		-- projection
		--local ortho = false
		--if obj.orthoProjection then 
			--ortho = true
		--local fov = obj.fov
		--local pos =  obj.position
		--local rot = obj.transform.rotation
		--local targetDistance = obj.targetDistance
		--local lookAt_ = $Camera001.Target
		
--	)
	
	--for obj in $* do(
		--local a = obj.name
		--format "obj % \n"  obj	
	--)

