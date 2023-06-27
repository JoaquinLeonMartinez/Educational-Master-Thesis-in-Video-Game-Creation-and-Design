
-- ----------------------------------------------------------
-- ----------------------------------------------------------
-- ----------------------------------------------------------
struct TMaterialExporter (
	
	fs = TJsonFormatter(), 
	project_path,                 -- Asigned in the ctor 
	base_path,                 -- Asigned in the ctor 	
	
	fn exportMap map alias default_value = (
		
		if map == undefined then (
			local json_filename = base_path + "textures/" + default_value + ".dds"
			fs.writeKeyValue alias json_filename
			return false
		)
		local current_scene_name = getFilenameFile maxFileName
		-- "C:\mcv\code\bin\data\textures\bricks.DDS"
		
		-- "C:\users\pep\desktop\download\bricks.dds"
		local map_filename = map.filename
		
		-- bricks
		local base_name = getFilenameFile map_filename
		
		-- data\textures\bricks.dds"
		local json_filename = base_path + "textures/" + current_scene_name + "/" + base_name + ".dds"
		
		-- 
		local ofull_path = project_path + json_filename
		
		-- Check if ofull_path exists
		if not doesFileExist ofull_path then (
			format "We should copy from % to %\n" map_filename ofull_path
			copyFile map_filename ofull_path
		)
		
		fs.writeKeyValue alias json_filename
	),
	
	fn isValidName aname = (
		-- Add no �, accents, etc.
		return findString aname " " == undefined 
	),
	
	-- Exports a single std material to a json format
	fn exportStdMaterial obj mat mat_name = (
		
		format "Exporting material % % %\n" mat_name mat (classof mat as string)
		
		if not (isValidName mat.name) then (
			throw ("Obj " + obj.name + " has a material with an invalid name " + mat.name )
		)	
		
		fs.begin (project_path + mat_name )
		fs.beginObj()
		  --Will use the defaults set in the parser. Needs to be a full data/techniques/textured.tech"
			local isDoor =  getUserProp obj "isDoor"
			local isPlatform = getUserProp obj "isPlatform"
			local aabb = getUserProp obj "AABB"
			local walltype = getUserProp obj "typeWall"
			local isPanel = getUserProp obj "isPanel"
			local isElectric = getUserProp obj "electric"
			local is_morphed = obj.modifiers[#Morpher]
			if isElectric == undefined and is_morphed == undefined and isPlatform == undefined and aabb == "static" and walltype == undefined and isPanel == undefined and isDoor == undefined then (
				fs.writeKeyValue "technique" "objs_culled_by_gpu.tech"
				fs.writeComma()
			)else (
				format "% wont be instantiated with % material is_morph: % isPlatform % aabb : % walltype: % isPaneL: %\n\n\n\n" obj.name mat.name is_morphed isPlatform aabb walltype isPanel
			)
			if is_morphed != undefined then(
				fs.writeKeyValue "technique" "objs_morph.tech"
				fs.writeComma()
				fs.writeKeyValue "category" "distorsions"
				fs.writeComma()
				fs.writeKeyValue "casts_shadows" false
				fs.writeComma()
			)

			fs.writeKey "textures" 
			fs.beginObj()
				
			if classof mat == Standardmaterial then (
				exportMap mat.diffuseMap "albedo" "null_albedo"
			    fs.writeComma()
				exportMap mat.bumpMap "normal" "null_normal"
				fs.writeComma()
				exportMap mat.selfIllumMap "emissive" "black"
				-- ... other maps..
			)else if classof mat == PhysicalMaterial then (
				exportMap mat.baseColorMap "albedo" "null_albedo"
			    fs.writeComma()
				exportMap mat.bumpMap "normal" "null_normal"
				fs.writeComma()
				if is_morphed != undefined then(
					fs.writeKeyValue "metallic" "acc_light.dds"
					fs.writeComma()
				) else (
					exportMap mat.metalnessMap "metallic" "black"
					fs.writeComma()
				)
				exportMap mat.roughnessMap "roughness" "white"
				fs.writeComma() 
				exportMap mat.emissionColorMap "emissive" "black"
			)
		
			fs.endObj()
		fs.endObj()
		fs.end()
	),
	
	-- Will return an array of all the materials names used by obj and exported by us
	fn exportMaterial mat base_name obj = (
		
		local exported_materials = #()
		if classof mat == StandardMaterial then (
			local mat_name = base_name + mat.name + ".material"
			append exported_materials mat_name
			exportStdMaterial obj mat mat_name
			
		) else if classof mat == PhysicalMaterial then (
		    local mat_name = base_name + mat.name + ".material"
			append exported_materials mat_name
			exportStdMaterial obj mat mat_name
			
		) else if classof mat == MultiMaterial then (
			local multi_mat = mat
			local materials_of_mesh = getMaterialsUsedByMesh obj
			for mat_idx = 1 to materials_of_mesh.count do (
				if materials_of_mesh[ mat_idx ] == undefined then continue
				local mat_of_mesh = multi_mat[ mat_idx ]
				
				if mat_of_mesh == undefined then throw ("Mesh " + obj.name + " is using a multimaterial in slot " + (mat_idx as string)+ " but the multimat does not have this submat")
				if classof mat_of_mesh != StandardMaterial and classof mat_of_mesh != PhysicalMaterial then throw ("Mesh " + obj.name + " is using a multimaterial in slot " + (mat_idx as string) + " but the multimat in this slot is not a stdMaterial, is "+(mat_of_mesh as string) + (mat_idx as string))
				
				local mat_name = base_name + mat_of_mesh.name + ".material"
				append exported_materials mat_name
				exportStdMaterial obj mat_of_mesh mat_name
			)
		)
		
		local is_fire = getUserProp obj "isFire"
		local is_fire_flame = getUserProp obj "isFireFlame"
		local isElectric = getUserProp obj "electric"
		if is_fire != undefined  then(
			exported_materials = #("data/materials/vfx/fire.material")
		)else if is_fire_flame != undefined then (
			exported_materials = #("data/materials/vfx/fire_flame.material")
		)else if isElectric != undefined then (
			exported_materials = #("data/materials/vfx/electric_panel.material")
		)
		
		return exported_materials
	)

	
)

--gc()
--me = TMaterialExporter project_path:"c:/code/engine/bin/" base_path:"data/"
--me.exportMaterial $.mat "data/materials/" $
