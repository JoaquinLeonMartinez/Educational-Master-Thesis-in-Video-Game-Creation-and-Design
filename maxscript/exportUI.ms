utility MyMCVExporter "MCV Exporter"
(
	
	rollout exporter "The Exporter" width:188 height:334
	(
		
		spinner 'curve_spn' "" pos:[137,31] width:33 height:16 range:[3,100,3] type:#integer align:#left
		button 'btn_curve' "Export Curve" pos:[51,26] width:80 height:21 align:#left
		spinner 'curve_transform_spn' "" pos:[138,60] width:33 height:16 range:[3,100,3] type:#integer align:#left
		button 'btn_curve_transform' "Export Curve Transform" pos:[13,57] width:117 height:21 align:#left
		button 'btn_scene' "Export Scene" pos:[54,85] width:80 height:21 align:#left
		button 'btn_mesh' "Export Mesh" pos:[56,111] width:76 height:21 align:#left
		button 'btn_prefab_atts' "Add Prefab Attributes" pos:[34,137] width:121 height:21 align:#left
		button 'btn_skeleton' "Export Skeleton & Meshes" pos:[24,163] width:141 height:21 align:#left
		button 'btn_skel_anims' "Export Animation" pos:[45,189] width:98 height:21 toolTip:"Uses max animation range and frame rate to specify the range to export" align:#left
		button 'btn_camera' "Export Camera" pos:[50,215] width:88 height:21 align:#left
		spinner 'curve_global_spn' "" pos:[135,245] width:33 height:16 range:[2,100,2] type:#integer align:#left
		button 'btn_GlobalCurve' "Export Global Curve" pos:[10,244] width:121 height:21 align:#left
		button 'btnwhiteboxit' "WhiteBox It" pos:[21,275] width:146 height:23 align:#left
		
		

		-- Exports the current selection mesh to file
		
		-- Exports the current selection mesh to file
		on btnwhiteboxit pressed do
		(
			try(
				local p = ConvertTo $ Editable_Poly
				polyop.setFaceSelection p #all
				polyOp.setFaceMatID p (polyOp.getFaceSelection p) 1
				MessageBox "DONE"
			)catch (
				MessageBox ("Error Whiteboxing :\n" + getCurrentException())
			)
		)
		on btn_curve pressed do
		(
			try(
				gc()
				local exporter = TCurveExporter()
				exporter.exportCurve $ curve_spn.value
				MessageBox "Single curve exported OK"
			) catch (
				MessageBox ("Error Exporting Curve:\n" + getCurrentException())
			)
		)
		on btn_curve_transform pressed do
		(
			try(
				gc()
				local exporter = TCurveTransformExporter()
				exporter.exportTransformCurve $ curve_transform_spn.value
				MessageBox "Transform curve exported OK"
			) catch (
				MessageBox ("Error Exporting Transform Curve:\n" + getCurrentException())
			)
		)
		on btn_scene pressed do
		(
			try (
				gc()
				local exporter = TSceneExporter()
				exporter.exportAll()	
				MessageBox "All exported OK"
			) catch (
				MessageBox ("Error Exporting:\n" + getCurrentException())
			)
		)
		on btn_mesh pressed do
		(
			
			try (
				gc()
				local ofilename = undefined  -- to be defined by you.
				exportMesh $ ofilename undefined
				MessageBox "Single mesh exported OK"
			) catch (
				MessageBox ("Error Exporting Single Mesh:\n" + getCurrentException())
			)
		)
		on btn_prefab_atts pressed do
		(
			if $ == undefined then return undefined
			custAttributes.add $ classPrefabData
		)
		on btn_skeleton pressed do
		(
			
			try (
				gc()
				local se = TSkeletonsExporter()
				se.exportSkelAndMeshes()
				MessageBox "Skeleton And Meshes exported OK"
			) catch (
				MessageBox ("Error Exporting Skeleton:\n" + getCurrentException())
			)
		)
		on btn_skel_anims pressed do
		(
			
			try (
				gc()
				local se = TSkeletonsExporter()
				se.exportAnim()
				MessageBox "Skeleton Animation exported OK"
			) catch (
				MessageBox ("Error Exporting Skeleton Animation:\n" + getCurrentException())
			)
		)
		on btn_camera pressed do
		(
			try (
				gc()
				local exporter = TCameraExporter()
				--format "en funcion createAllCamera to %\n " $
				exporter.createAllCamera()
				
				MessageBox "All exported OK"
			) catch (
				MessageBox ("Error Exporting:\n" + getCurrentException())
			)
			
		)
		on btn_GlobalCurve pressed do
		(
			try(
				gc()
				local exporter = TCurveGlobalExporter()
				exporter.exportGlobalCurve $ curve_global_spn.value
				MessageBox "Global curve exported OK"
			) catch (
				MessageBox ("Error Exporting Curve:\n" + getCurrentException())
			)
		)
	)
		
	-- ...
	on MyMCVExporter open do
	(
		addRollout exporter
	) 
	on MyMCVExporter close do
	(
		removeRollout exporter
	) 
) -- end utility M