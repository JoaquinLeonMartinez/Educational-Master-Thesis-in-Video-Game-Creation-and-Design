SLB.using(SLB)

GameController = getGameController()

--variables globales
h_suishi1 = nil
h_suishi2 = nil
h_suishi3 = nil
h_suishi4 = nil
h_suishi5 = nil
h_suishi6 = nil
h_suishi7 = nil


h_suishi9 = nil
h_suishi10 = nil
h_suishi11 = nil
h_suishi12 = nil
h_suishi13 = nil
h_suishi14 = nil
h_suishi15 = nil



--funciones
function on_heal_player()	
	GameController:healPlayer()
end

function on_restore_madness()
	GameController:restoreMadness()
end

function set_pause_enemy_by_handle(handle,pause)
	GameController:setPauseEnemyByHandle(handle,pause)
end

function delete_grietas(name)
	on_deleteElement(name)
end

function on_spawn_prefab(name)
	GameController:spawnPrefab(name, Vector3(0.0, 0.0, 0.0))
end

function on_god_mode(active)
	GameController:setGodMode(active)
end

function on_active_enemies(active)
	if active then
		GameController:resumeEnemies()
	else
		GameController:stopEnemies()
	end
end

function setNotThrowCupcake(nameGolem,active)
	handle = GameController:entityByName(nameGolem);
	t_compGolem = toCBTGolem(toEntity(handle):getCompByName("bt_golem"));
	t_compGolem:setNotThrowCupcake(active);
end

function on_gameplay_fragment_activate(modl)
	GameController:activateGameplayFragment(modl)
end

function on_gameplay_fragment_deactivate(modl)
	GameController:deactivateGameplayFragment(modl)
end

function on_salute()
	salute()
end

function on_activate_spawner(id)
	GameController:activateSpawner(id)
end

function on_wake_sushi(id)
	GameController:wakeUpSushi(id)
end

function on_deleteElement(id)
	GameController:deleteElement(id)
end

function blendPlayerCamera()
	GameController:blendPlayerCamera()
end


function setAmbient(value)
	GameController:setAmbient(value)
end

function on_ambush_event_1()
	on_wake_sushi("002")
	on_wake_sushi("003")
	on_deleteElement("pisoFragmento01")
	on_deleteElement("pisoFragmento02")
	on_deleteElement("pisoFragmento03")
	on_activate_spawner("Spawner_dumpling_01")
	on_activate_spawner("Spawner_dumpling_002")
	on_activate_spawner("Spawner_dumpling_003")

end

function on_ambush_event_2()

end


function setPauseEnemyName(name,state)	
	GameController:setPauseEnemyByName(name,state);
end


--MILESTONE 3
--zona panaderia

function on_create_enemies_zone_cupcake_player()


	
	execDelayedAction("changeScene(\"tutorial_scene\")",0)	

	--h_cupcake1 = GameController:spawnPrefab("data/prefabs/enemies/bt_cupcake.json", VEC3(183, -48.460, -66), QUAT(0, 0, 0, 1),1);
	--GameController:updateCupcakeCurveByHandle("curvaCupcakePanaderia1",h_cupcake1);
	--GameController:setPauseEnemyByHandle(h_cupcake1,false);
	--GameController:setLifeEnemy(h_cupcake1,3,350.0);
	
	h_cupcake2 = GameController:spawnPrefab("data/prefabs/enemies/bt_cupcake.json", VEC3(175, -48.460, -72), QUAT(0, 0, 0, 1),1);
	GameController:updateCupcakeCurveByHandle("curvaCupcakePanaderia2",h_cupcake2);
	GameController:setPauseEnemyByHandle(h_cupcake2,false);
	GameController:setLifeEnemy(h_cupcake2,3,350.0);

	h_cupcake3 = GameController:spawnPrefab("data/prefabs/enemies/bt_cupcake.json", VEC3(184, -48.460, -74), QUAT(0, 0, 0, 1),1);
	GameController:updateCupcakeCurveByHandle("curvaCupcakePanaderia3",h_cupcake3);
	GameController:setPauseEnemyByHandle(h_cupcake3,false);
	GameController:setLifeEnemy(h_cupcake3,3,350.0);

	
	h_oven1 =  GameController:entityByName("horno1");
	h_oven2 =  GameController:entityByName("horno5");
	h_oven3 =  GameController:entityByName("horno6");
	h_oven4 =  GameController:entityByName("horno007");

	
	t_compSpawnOven1 = toCompEnemySpawner(toEntity(h_oven1):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven2 = toCompEnemySpawner(toEntity(h_oven2):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven3 = toCompEnemySpawner(toEntity(h_oven3):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven4 = toCompEnemySpawner(toEntity(h_oven4):getCompByName("comp_enemy_spawner"));

	t_compSpawnOven1:setLifeSpawner(350.0);
	t_compSpawnOven2:setLifeSpawner(350.0);
	t_compSpawnOven3:setLifeSpawner(350.0);
	t_compSpawnOven4:setLifeSpawner(350.0);


	t_compSpawnOven1:setScriptTriggerActivate(true);
	t_compSpawnOven1:setCurveForSpawner("curvaCupcakePanaderia1");



	execDelayedAction("on_delete_handle(\"triggerCreacionCupcackes\")",0);

	--Audio
	GameController:updateSoundtrackID(2);
end


--Zona Suishy script parte Cinematica + asignacion de curvas + despertar suishis

function on_blending_camera(name,speed,typeInterpolator)
	GameController:blendingCamera(name,speed,typeInterpolator)
end

function on_GPUdeleteScene(name)
	GameController:GPUdeleteScene(name)
end

function on_load_gpu_scene(name)
	GameController:GPUloadScene(name)
end

function on_lock_camera3(activate)
	GameController:lockCamera3Person(activate)
end

function on_delete_handle(name)
	GameController:destroyCHandleByName(name)
end

function on_cinematic(flag)
	GameController:inCinematic(flag)
end

function on_cinematic_special(flag,type)
	GameController:inCinematicSpecial(flag,type)
end


function on_cinematic_golem(name,flag)
	GameController:inCinematicGolem(name,flag)
end

function destroy_and_wake_up(name_golem,name_wall,intensity)
	-- OLD: GameController:destroyWallByName(name_wall,name_golem, intensity) -- rompe el muro de delante del golem
	GameController:wakeUpGolem(name_golem) -- despierta al golem
end

function setViewDistanceEnemy(distance,handle,type)
	GameController:setViewDistanceEnemyByHandle(distance,handle,type);
end


function sethalfConeEnemy(halfCone,handle,type)
	GameController:setHalfConeEnemyByHandle(halfCone,handle,type)
end

function on_load_gpu_products(filename)
	GameController:loadProducts(filename);
end

function on_delete_gpu_products()
	GameController:deleteProducts();
end

function script_ice_1_player()
	
	--execDelayedAction("changeScene(\"congelados_scene\")",0)	
	--GameController:resetCamera();  comentado por dani para entrega final
	execDelayedAction("on_cinematic(true)",0);
	execDelayedAction("on_lock_camera3(false)",0);
	execDelayedAction("on_blending_camera(\"CameraPanel001\", 5,\"Quadin\")",0);
	--execDelayedAction("on_GPUdeleteScene(\"data/scenes/mapa_panaderia.json\")",3);
	--execDelayedAction("on_load_gpu_products(\"data/scenes/mapa_asiatica.json\")",8.0);
	execDelayedAction("childAppears(\"MISION_3\",true,true,0.0,1.25)", 6.1);
	execDelayedAction("on_blending_camera(\"CameraPanel002\", 5,\"Quadin\")",6);	
    execDelayedAction("destroy_and_wake_up(\"golem2\",\"Box007\", 20)",15); 
	execDelayedAction("on_blending_camera(\"PlayerCamera\", 5,\"Quadin\")",12);
	execDelayedAction("on_cinematic(false)",16);
	execDelayedAction("on_lock_camera3(true)",16);
	handle = GameController:entityByName("golem2");
	execDelayedAction("setViewDistanceEnemy(80,handle,4)",15.5);
	GameController:setHeightEnemyByHandle(7.0,handle,4);
	execDelayedAction("on_delete_handle(\"trigger001\")",0);
	--Audio
	GameController:updateSoundtrackID(3);
end

function script_ice_2_player()

	--GameController:destroyWallByName("bx4", "golem2", 20) 
	GameController:wakeUpGolem("golem2")
	GameController:sleepGolem("golem1")
	-- despierta al segundo golem y rompe el segundo muro
	-- funcion para despertar al golem y que fije al player pah siempre
	-- funcion que rompa el muro en varios trozos

	execDelayedAction("on_delete_handle(\"trigger002\")",0);
end

function script_ice_3_player()
	execDelayedAction("setPauseEnemyName(\"golem3\",true)", 0);
	execDelayedAction("on_delete_handle(\"trigger003\")",0);
end

function create_cupcakes_in_frost_player()
	--GameController:deleteCupcake();

	execDelayedAction("changeScene(\"congelados_scene\")",0)
	--h_cupcake1 = GameController:spawnPrefab("data/prefabs/enemies/bt_cupcake.json", VEC3(149, -3.653, -52), QUAT(0, 0, 0, 1),1);
	--GameController:updateCupcakeCurveByHandle("curvaCupcakeCongelados1",h_cupcake1);
	--GameController:setPauseEnemyByHandle(h_cupcake1,false);
	--GameController:setLifeEnemy(h_cupcake1,3,350.0);


	
	--h_cupcake2 = GameController:spawnPrefab("data/prefabs/enemies/bt_cupcake.json", VEC3(156, -3.653, -59), QUAT(0, 0, 0, 1),1);
	--GameController:updateCupcakeCurveByHandle("curvaCupcakeCongelados2",h_cupcake2);
	--GameController:setPauseEnemyByHandle(h_cupcake2,false);
	--GameController:setLifeEnemy(h_cupcake2,3,350.0);

	--execDelayedAction("changeIntensityLightSpot(\"SpotJoint\",0.0)",0.0);--prueba 

	h_oven1 =  GameController:entityByName("horno008");
	h_oven2 =  GameController:entityByName("horno009");
	h_oven3 =  GameController:entityByName("horno010");

	
	t_compSpawnOven1 = toCompEnemySpawner(toEntity(h_oven1):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven2 = toCompEnemySpawner(toEntity(h_oven2):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven3 = toCompEnemySpawner(toEntity(h_oven3):getCompByName("comp_enemy_spawner"));
	

	t_compSpawnOven1:setLifeSpawner(350.0);
	t_compSpawnOven2:setLifeSpawner(350.0);
	t_compSpawnOven3:setLifeSpawner(350.0);
	

	playAnnouncement('event:/UI/Announcements/Announcement4');

	execDelayedAction("on_delete_handle(\"trigger010\")",0);

	--execDelayedAction("on_lock_camera3(false)",0);
	--execDelayedAction("on_delete_gpu_products()",1);
	--execDelayedAction("on_lock_camera3(true)",2);
end

function crearTrampaHornos()

	--GameController:resetCamera();
	--execDelayedAction("on_cinematic(true)",0.0);
	--execDelayedAction("on_lock_camera3(false)",0.0);
	--execDelayedAction("on_blending_camera(\"CameraPanaderiaPlat\", 5,\"linear\")",0.1);
	--execDelayedAction("on_blending_camera(\"PlayerCamera\",5,\"linear\")",13.5);
	--execDelayedAction("on_lock_camera3(true)",18);
	--execDelayedAction("on_cinematic(false)",18);
	

	h_oven1 =  GameController:entityByName("horno008");
	h_oven2 =  GameController:entityByName("horno009");
	h_oven3 =  GameController:entityByName("horno010");
	
	t_compSpawnOven1 = toCompEnemySpawner(toEntity(h_oven1):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven2 = toCompEnemySpawner(toEntity(h_oven2):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven3 = toCompEnemySpawner(toEntity(h_oven3):getCompByName("comp_enemy_spawner"));
	
	t_compSpawnOven1:setLifeSpawner(50.0);
	t_compSpawnOven2:setLifeSpawner(50.0);
	t_compSpawnOven3:setLifeSpawner(50.0);
	

	GameController:setLifeEnemiesByTag("cupcake",50.0);


	t_compSpawnOven1:setComportamentNormal(1);
	t_compSpawnOven2:setComportamentNormal(1);
	t_compSpawnOven3:setComportamentNormal(1);


	GameController:spawnPrefab("data/prefabs/structures/trigger_special_oven.json", VEC3(96,-3.653, -59), QUAT(0, 0, 0, 1),1);


end

function changeIntensityLightOmni(nameLight,intensity)
	h_light =  GameController:entityByName(nameLight);
	t_comp_light_point = toCompLightPoint(toEntity(h_light):getCompByName("light_point"));
	t_comp_light_point:setIntensity(intensity)
end


function changeIntensityLightSpot(nameLight,intensity)
	h_light =  GameController:entityByName(nameLight);
	t_comp_light_dir = toCompLightDir(toEntity(h_light):getCompByName("light_dir"));
	t_comp_light_dir:setIntensity(intensity)
end

function changeShadowsEnabled(nameLight,enabled)
	h_light =  GameController:entityByName(nameLight);
	t_comp_light_dir = toCompLightDir(toEntity(h_light):getCompByName("light_dir"));
	t_comp_light_dir:setShadowEnabled(enabled)

end

function changeShadowsEnabledJoint(value)
	GameController:changeShadowsEnabledJoint(value);
end

function changeLightsIntensityJoint(value)
	GameController:changeLightsIntensityJoint(value);
end


function wake_up_last_golem_player()
	h_golem = GameController:entityByName("golem3"); 
	execDelayedAction("setPauseEnemyName(\"golem3\",false)", 0);
	GameController:setHeightEnemyByHandle(10.0,h_golem,4);
	execDelayedAction("balanceoLampara(\"Joint001\")",0);


	
	--execDelayedAction("changeIntensityLightSpot(\"Spot001\",0.0)",1.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot002\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot003\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot004\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot005\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot006\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot007\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot008\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot009\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot010\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot011\",0.0)",2.0);
	--execDelayedAction("changeIntensityLightSpot(\"Spot014\",0.0)",2.0);

	execDelayedAction("changeShadowsEnabled(\"Spot001\",false)",1.1);
	execDelayedAction("changeShadowsEnabled(\"Spot002\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot003\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot004\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot005\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot006\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot007\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot008\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot009\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot010\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot011\",false)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot014\",false)",2.1);

	execDelayedAction("changeShadowsEnabled(\"Spot016\",true)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot017\",true)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot018\",true)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot019\",true)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot020\",true)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot021\",true)",2.1);
	execDelayedAction("changeShadowsEnabled(\"Spot022\",true)",2.1);
	

	execDelayedAction("changeShadowsEnabledJoint(true)",2.2);
	


	--execDelayedAction("on_delete_handle(\"trigger007\")",0);
end

function in_trap_tube_enemies_player()
	
	execDelayedAction("playAnnouncement(\"event:/UI/Announcements/Announcement1\")",3.0);

	--GameController:resetCamera();
	h_golem = GameController:entityByName("golem3"); 
	execDelayedAction("setPauseEnemyName(\"golem3\",true)", 0);	
	execDelayedAction("on_cinematic(true)",3);
	execDelayedAction("on_lock_camera3(false)",3);
	execDelayedAction("on_blending_camera(\"CameraEnemiesTube\", 5,\"linear\")",3); --poner en mapa la cmara correspondiente a donde estaran ubicados los termoestatos
	--execDelayedAction("on_load_gpu_products(\"data/scenes/mapa_asiatica.json\")",5);
	execDelayedAction("on_blending_camera(\"PlayerCamera\", 2,\"linear\")",7);
	execDelayedAction("on_lock_camera3(true)", 9);
	execDelayedAction("on_cinematic(false)",9);
	

	handle = GameController:entityByName("enemies_in_tube");--prefab
	t_compenemiestube = toCompEnemiesInTube(toEntity(handle):getCompByName("enemies_in_tube"));
	t_compenemiestube.activateTrap = true
	
	--execDelayedAction("balanceoLampara(\"Joint001\")",9);
	execDelayedAction("on_delete_handle(\"trigger008\")",0);



end





function cinematic_scene_termoestatos_player()
	--evento cinematica
	playAnnouncement('event:/UI/Announcements/Announcement9')

	GameController:resetCamera();
	execDelayedAction("on_cinematic(true)",0.1);
	execDelayedAction("on_lock_camera3(false)",0);
	execDelayedAction("on_blending_camera(\"CameraTermoestato1\", 5,\"linear\")",0); --poner en mapa la cmara correspondiente a donde estaran ubicados los termoestatos
	execDelayedAction("childAppears(\"MISION_5\",true,true,0.0,1.25)", 4.5);
	execDelayedAction("on_blending_camera(\"CameraTermoestato2\", 5,\"linear\")",7); --volver a la camara del jugador
	execDelayedAction("on_blending_camera(\"PlayerCamera\", 5,\"linear\")",14);
	execDelayedAction("childAppears(\"MISION_6\",true,true,0.0,1.25)", 13.25)
	execDelayedAction("on_lock_camera3(true)",19);
	execDelayedAction("on_cinematic(false)",19);
	--asignar curvas a enemigos
	--execDelayedAction("curve_to_enemy_zone_first_floor()",0)
	--despertar enemigos
	--execDelayedAction("active_sushi_asiatic_zone_first_floor()",0)

	--prueba de cargar la sgunda escena
	--execDelayedAction("changeScene(\"tutorial_scene\")",0);--Prueba de cambio de escena
	--eliminar trigger de animacion
	execDelayedAction("setViewDistanceEnemy(1000,h_suishi1,1)",19);
	execDelayedAction("sethalfConeEnemy(360,h_suishi1,1)",19);
	
	execDelayedAction("sethalfConeEnemy(360,h_suishi4,2)",19);
	execDelayedAction("setViewDistanceEnemy(1000,h_suishi4,2)",19);
	
	execDelayedAction("sethalfConeEnemy(360,h_suishi6,2)",19);
	execDelayedAction("setViewDistanceEnemy(1000,h_suishi6,2)",19);
	
	execDelayedAction("sethalfConeEnemy(360,h_suishi9,1)",19);
	execDelayedAction("setViewDistanceEnemy(70,h_suishi9,1)",19);
	
	execDelayedAction("sethalfConeEnemy(360,h_suishi10,1)",19);
	execDelayedAction("setViewDistanceEnemy(70,h_suishi10,1)",19);
	
	execDelayedAction("sethalfConeEnemy(360,h_suishi11,2)",19);
	execDelayedAction("setViewDistanceEnemy(70,h_suishi11,2)",19);

	
	

	execDelayedAction("on_delete_handle(\"trigger3\")",0);
	--execDelayedAction("spawn(\"data/prefabs/enemies/sushi.json\", VEC3(-10, 0, -43.5), QUAT(0, 0, 0, 1),1)",0);
	
	--Suishis piso inferior
	
	--Audio
	GameController:updateSoundtrackID(4);
end



function createEnemies_player()
	--execDelayedAction("balanceoLampara(\"Joint001\")",0);
	GameController:deleteGolem("golem2");
	GameController:deleteGolem("golem3");
	GameController:deleteCupcake();
	execDelayedAction("changeScene(\"asiatic_scene\")",0);

	execDelayedAction("on_delete_handle(\"trigger2\")",0);	

	h_suishi1 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-33,-0.193,-52), QUAT(0, 0, 0, 1),1);
	GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica1",h_suishi1);
	GameController:setPauseEnemyByHandle(h_suishi1,false);
	
	--h_suishi2 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-40,-0.193,-62), QUAT(0, 0, 0, 1),1);
	--GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica2",h_suishi2);
	--GameController:setPauseEnemyByHandle(h_suishi2,false);

	
	h_suishi4 = GameController:spawnPrefab("data/prefabs/enemies/bt_ranged_sushi.json", VEC3(-2,-0.193, -73), QUAT(0, 0, 0, 1),1);
	GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica4",h_suishi4);
	GameController:setPauseEnemyByHandle(h_suishi4,false);
	

	h_suishi6 = GameController:spawnPrefab("data/prefabs/enemies/bt_ranged_sushi.json", VEC3(-14,-0.193, -65), QUAT(0, 0, 0, 1),1);
	GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica6",h_suishi6);
	GameController:setPauseEnemyByHandle(h_suishi6,false);

	--h_suishi7 = GameController:spawnPrefab("data/prefabs/enemies/bt_ranged_sushi.json", VEC3(-40,-0.193,-80), QUAT(0, 0, 0, 1),1);
	--GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica7",h_suishi7);
	--GameController:setPauseEnemyByHandle(h_suishi7,false);


	--suishis piso superior (Quizas setear el height para que no te ataquen antes de tiempo)
	h_suishi9 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-47,7.632,-18), QUAT(0, 0, 0, 1),1);
	GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica9",h_suishi9);
	GameController:setHeightEnemyByHandle(0,h_suishi9,1);
	GameController:setPauseEnemyByHandle(h_suishi9,false);

	h_suishi10 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-53,7.632,-17), QUAT(0, 0, 0, 1),1);
	GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica10",h_suishi10);
	GameController:setHeightEnemyByHandle(0,h_suishi10,1);
	GameController:setPauseEnemyByHandle(h_suishi10,false);

	h_suishi11 = GameController:spawnPrefab("data/prefabs/enemies/bt_ranged_sushi.json", VEC3(-60,7.632,-23), QUAT(0, 0, 0, 1),1);
	GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica11",h_suishi11);
	GameController:setHeightEnemyByHandle(0,h_suishi11,2);
	GameController:setPauseEnemyByHandle(h_suishi11,false);

	GameController:setViewDistanceEnemy(0,h_suishi9,1);
	GameController:setViewDistanceEnemy(0,h_suishi10,1);
	GameController:setViewDistanceEnemy(0,h_suishi11,2);

	--h_suishi12 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(80,7.632,137), QUAT(0, 0, 0, 1),1);--quitar este
	--GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica12",h_suishi12);
	--GameController:setPauseEnemyByHandle(h_suishi12,true); --GameController:setPauseEnemyByHandle(h_suishi12,true);

	--suishis que no te atacan hasta el derretir el 2do termoestato

	--h_suishi13 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-73,-0.193,-91), QUAT(0, 0, 0, 1),1);
	--GameController:setPauseEnemyByHandle(h_suishi13,true);
	--h_suishi14 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-58,-0.193,-98), QUAT(0, 0, 0, 1),1);
	--GameController:setPauseEnemyByHandle(h_suishi14,true);
	--h_suishi15 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(71,-0.193,61), QUAT(0, 0, 0, 1),1);--quitar este
	--GameController:setPauseEnemyByHandle(h_suishi15,true); --GameController:setPauseEnemyByHandle(h_suishi15,true);
	playAnnouncement('event:/UI/Announcements/Announcement6');
end


function viewAtPlayerSushisZanahoria_player()
	GameController:setViewDistanceEnemyByHandle(60,h_suishi9,1);
	GameController:setViewDistanceEnemyByHandle(60,h_suishi10,1);
	GameController:setViewDistanceEnemyByHandle(60,h_suishi11,2);
	GameController:setHeightEnemyByHandle(1,h_suishi9,1);
	GameController:setHeightEnemyByHandle(1,h_suishi10,1);
	GameController:setHeightEnemyByHandle(1,h_suishi11,2);
	
	execDelayedAction("on_delete_handle(\"trigger1\")",0);
end



function setTransformObject_(name,pos,yaw,pith,roll)
	GameController:setTransformObject(name,pos,yaw,pith,roll);
	--GameController:destroyCHandleByName(name)
end




function wakeUpSuishisSecondFloor()
	if h_suishi9 ~= nil then --con uno vale, ya que se instancian a la vez
		
		GameController:setPauseEnemyByHandle(h_suishi9,false);
		
		GameController:setPauseEnemyByHandle(h_suishi10,false);
	
		GameController:setPauseEnemyByHandle(h_suishi11,false);
		
		--execDelayedAction("saveCheckpoint()",0.1);


		--GameController:setPauseEnemyByHandle(h_suishi12,false);
	end
end

function defrost2()
	
	--execDelayedAction("changeLookAt(\"cubosHielo_033\",0.5)",0);
	--execDelayedAction("on_delete_handle(\"cubosHielo_033\")",5);--simulamos tiempo de animacion morph por ahora
	--execDelayedAction("changeLookAt(\"cubosHielo_033\",0)",4.9);
	--GameController:destroyCHandleByName(name)


	--EN TEORIA ESTO DE AQUI CUANDO ESTE ACABADA LA ESCENA DESCOMENTARLO Y COMENTAR LAS LINEAS DE ARRIBA
	execDelayedAction("changeLookAt(\"cubosHielo_033\",0.5)",0);
	execDelayedAction("playMorph(\"cubosHielo_033\")",0);
	execDelayedAction("changeLookAt(\"cubosHielo_033\",0)",4.9);
	execDelayedAction("on_delete_handle(\"cubosHielo_033\")",5.1);

	GameController:spawnPrefab("data/particles/hot_particles.json", VEC3(0,0,0), QUAT(0, 0, 0, 1),1);
	GameController:spawnPrefab("data/particles/platform_fire_particles.json", VEC3(0,0,0), QUAT(0, 0, 0, 1),1);
	GameController:spawnPrefab("data/particles/outside_fire1.json", VEC3(0,0,0), QUAT(0, 0, 0, 1),1);
	GameController:spawnPrefab("data/particles/outside_fire2.json", VEC3(0,0,0), QUAT(0, 0, 0, 1),1);


end

function changeLookAt(name,amount)
	handle = GameController:getPlayerHandle();
	t_compskellook = toCompSkelLookAt(toEntity(handle):getCompByName("skel_lookat"));
	t_compskellook.target_entity_name = name
	t_compskellook.amount= amount
	t_compskellook.flagFirst = true--nuevo, esto es para el giro inicial de cabeza
end

function playMorph(name)
	GameController:playAnimationMorph(name)
end

function stopMorph(name)
	GameController:stopAnimationMorph(name)
end

function defrost1()
	--execDelayedAction("changeLookAt(\"cubosHielo_034\",0.5)",0);
	--execDelayedAction("on_delete_handle(\"cubosHielo_034\")",5);--simulamos tiempo de animacion morph por ahora
	--execDelayedAction("changeLookAt(\"cubosHielo_034\",0)",4.9);

	--EN TEORIA ESTO DE AQUI CUANDO ESTE ACABADA LA ESCENA DESCOMENTARLO Y COMENTAR LAS LINEAS DE ARRIBA
	execDelayedAction("changeLookAt(\"cubosHielo_034\",0.5)",0);
	execDelayedAction("playMorph(\"cubosHielo_034\")",0);
	execDelayedAction("changeLookAt(\"cubosHielo_034\",0)",4.9);
	execDelayedAction("on_delete_handle(\"cubosHielo_034\")",5.1);
end

function on_spawnPrefab(prefab,pos,quat,scale)
	handle = GameController:spawnPrefab(name, pos, quat,scale);
	GameController:setPauseEnemyByHandle(handle,false)
end

function on_ambush()
	--suishis que se despiertan al romper ultimo termoestato
	
	h_suishi13 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-73,-0.193,-91), QUAT(0, 0, 0, 1),1);
	GameController:setPauseEnemyByHandle(h_suishi13,false);
	h_suishi14 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-58,-0.193,-98), QUAT(0, 0, 0, 1),1);
	GameController:setPauseEnemyByHandle(h_suishi14,false);
	GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica13",h_suishi13);
	GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica14",h_suishi14);

	execDelayedAction("sethalfConeEnemy(360,h_suishi13,1)",0);
	execDelayedAction("setViewDistanceEnemy(100,h_suishi13,1)",0);

	execDelayedAction("sethalfConeEnemy(360,h_suishi14,1)",0);
	execDelayedAction("setViewDistanceEnemy(100,h_suishi14,1)",0);


	execDelayedAction("saveCheckpoint()",0.1);
		
		--execDelayedAction("saveCheckpoint()",0.1);
		--GameController:updateEnemyCurveByHandle("curvaSuihiAsiatica15",h_suishi15);
		--GameController:setPauseEnemyByHandle(h_suishi15,false);
	end


function activePlatformByName(name)
	GameController:activatePlatformByName(name);
end

--MILESTONE 3  zona madalenas Plataforma activada

function activePlataform_player()--activacion de la plataforma zona final
	--GameController:activatePlatformByName("plat1346467");
	execDelayedAction("activePlatformByName(\"plat1346467\")",0.0);
end

--ZONA CARNICERIA
function changeCurvePlatform(nameCurve,namePlatform)
	GameController:updatePlatformCurveByName(nameCurve,namePlatform);
end


function changeIntensityLightPointWithFlickering(nameLight,intensity)
	h_light =  GameController:entityByName(nameLight);
	t_comp_flickering = toCompFlickering(toEntity(h_light):getCompByName("flickering"));
	t_comp_flickering:setBase(intensity)--el base del flickering manda sobre el instensisy del componente pointLight
	t_comp_flickering:setFrequency(0)
	t_comp_flickering:setPhase(0)
end



--activar plataformas en torre carniceria y creacion de suishis
function activePlataformCarniceria_player()
	
	GameController:deleteSushi();


	execDelayedAction("changeScene(\"carniceria_scene\")",0);
	--execDelayedAction("changeCurvePlatform(\"curvaTrampaPinchos001_v2\",\"trampa_pinchos001\")",0);
	--execDelayedAction("changeCurvePlatform(\"curvaTrampaPinchos002_v2\",\"trampa_pinchos002\")",0);
	--execDelayedAction("changeCurvePlatform(\"curvaTrampaPinchos003_v2\",\"trampa_pinchos003\")",0);
	--execDelayedAction("changeCurvePlatform(\"curvaTrampaPinchos004_v2\",\"trampa_pinchos004\")",0);
	--execDelayedAction("activePlatformByName(\"trampa_pinchos001\")",0.1);
	--execDelayedAction("activePlatformByName(\"trampa_pinchos002\")",0.3);
	--execDelayedAction("activePlatformByName(\"trampa_pinchos003\")",0.5);
	--execDelayedAction("activePlatformByName(\"trampa_pinchos004\")",0.7);
	execDelayedAction("activePlatformByName(\"trap001\")",0);
	execDelayedAction("activePlatformByName(\"trap002\")",0.5);
	execDelayedAction("activePlatformByName(\"trap003\")",0.3);
	execDelayedAction("on_delete_handle(\"triggerActivarPlataformas\")",0);
	--suishis PB
	h_suishi1 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-127,0.316,-228), QUAT(0, 0, 0, 1),1);
	GameController:updateEnemyCurveByHandle("CurvaSuishiCarn1",h_suishi1);
	GameController:setPauseEnemyByHandle(h_suishi1,true);
	
	--h_suishi2 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-121,0.316,-190), QUAT(0, 0, 0, 1),1);
	--GameController:updateEnemyCurveByHandle("CurvaSuishiCarn2",h_suishi2);
	--GameController:setPauseEnemyByHandle(h_suishi2,true);

	h_suishi3 = GameController:spawnPrefab("data/prefabs/enemies/bt_ranged_sushi.json", VEC3(-121,0.316,-190), QUAT(0, 0, 0, 1),1);
	GameController:updateEnemyCurveByHandle("CurvaSuishiCarn3",h_suishi3);
	GameController:setPauseEnemyByHandle(h_suishi3,true);
	
	h_suishi4 = GameController:spawnPrefab("data/prefabs/enemies/bt_ranged_sushi.json", VEC3(-121,0.316,-218), QUAT(0, 0, 0, 1),1);
	GameController:updateEnemyCurveByHandle("CurvaSuishiCarn4",h_suishi4);
	GameController:setPauseEnemyByHandle(h_suishi4,true);
	
	
	--h_suishi5 = GameController:spawnPrefab("data/prefabs/enemies/bt_sushi.json", VEC3(-121,0.316,-218), QUAT(0, 0, 0, 1),1);
	--GameController:updateEnemyCurveByHandle("CurvaSuishiCarn5",h_suishi5);
	--GameController:setPauseEnemyByHandle(h_suishi5,true);
	
	trap_sushis = GameController:spawnPrefab("data/prefabs/traps/enemies_in_butcher.json", VEC3(33,100,-65), QUAT(0, 0, 0, 1),1);

	--Audio
	GameController:updateSoundtrackID(5);
	-- CAMERA LOCA
	execDelayedAction("cinematica_tower()",0.0);

	
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni038\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni039\",0.0)",15.5);
	
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni040\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni041\",0.0)",15.5);
	
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni042\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni043\",0.0)",15.5);

	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni044\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni035\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni034\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni033\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni032\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni031\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni030\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni029\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni028\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni027\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni026\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni025\",0.0)",15.5);
	--execDelayedAction("changeIntensityLightPointWithFlickering(\"Omni024\",0.0)",15.5);
	
	execDelayedAction("changeIntensityLightOmni(\"Omni038\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni039\",0.0)",15.5);
	
	execDelayedAction("changeIntensityLightOmni(\"Omni040\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni041\",0.0)",15.5);
	
	execDelayedAction("changeIntensityLightOmni(\"Omni042\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni043\",0.0)",15.5);

	execDelayedAction("changeIntensityLightOmni(\"Omni044\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni035\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni034\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni033\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni032\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni031\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni030\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni029\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni028\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni027\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni026\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni025\",0.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni024\",0.0)",15.5);



	execDelayedAction("playAnnouncement(\"event:/UI/Announcements/Announcement11\")",15.0);
	
	GameController:setAmbient(0.05);

	
	execDelayedAction("set_pause_enemy_by_handle(h_suishi1,false)",15.8);
	execDelayedAction("set_pause_enemy_by_handle(h_suishi3,false)",15.8);
	execDelayedAction("set_pause_enemy_by_handle(h_suishi4,false)",15.8);
	--execDelayedAction("set_pause_enemy_by_handle(h_suishi5,false)",12.5);
	execDelayedAction("setViewDistanceEnemy(1000,h_suishi1,1)",15.0);
	execDelayedAction("sethalfConeEnemy(360,h_suishi1,1)",15.0);
	--execDelayedAction("setViewDistanceEnemy(1000,h_suishi2,1)",12.5);
	--execDelayedAction("sethalfConeEnemy(360,h_suishi2,1)",12.5);
	execDelayedAction("setViewDistanceEnemy(1000,h_suishi3,2)",15.0);
	execDelayedAction("sethalfConeEnemy(360,h_suishi3,2)",15.0);
	execDelayedAction("setViewDistanceEnemy(1000,h_suishi4,2)",15.0);
	execDelayedAction("sethalfConeEnemy(360,h_suishi4,2)",15.0);
	--execDelayedAction("setViewDistanceEnemy(1000,h_suishi5,1)",12.5);
	--execDelayedAction("sethalfConeEnemy(360,h_suishi5,1)",12.5);
	execDelayedAction("childAppears(\"MISION_8\",true,true,0.0,1.25)", 15.8)
	execDelayedAction("saveCheckpoint()",16);


	
end



--activar ascensor final
function activeElevatorFinishCarniceria_player()
	execDelayedAction("activePlatformByName(\"ascensor002\")",0.3);
	execDelayedAction("on_delete_handle(\"triggerActivarAscensorFinal\")",0);
end
--activar ascensor inicial
function activeElevatorInitCarniceria_player()
	execDelayedAction("activePlatformByName(\"ascensor\")",0.3);
	execDelayedAction("on_delete_handle(\"triggerActivarAscensorInicial\")",0);
end


function caidaSuishisPisos_enemy(name)
	--matar/cambiar curvas seg�n el sushi que cae
	
	if name == "Sushi004" then
		h_suishi4 = GameController:entityByName("Sushi004");
		GameController:updateEnemyCurveByHandle("CurvaSuishiCarn1",h_suishi4);
	end
	if name == "Sushi005" then
		h_suishi5 = GameController:entityByName("Sushi005");
		GameController:updateEnemyCurveByHandle("CurvaSuishiCarn2",h_suishi5);
	end
	if name == "Sushi006" then
		h_suishi6 = GameController:entityByName("Sushi006");
		GameController:updateEnemyCurveByHandle("CurvaSuishiCarn3",h_suishi6);
	end
end

function trampaSushisButcher_player()
	
	--trap_sushis = GameController:spawnPrefab("data/prefabs/traps/enemies_in_butcher.json", VEC3(33,100,-65), QUAT(0, 0, 0, 1),1);

	--execDelayedAction("saveCheckpoint()",0.0);
	--por si queremos poner cinematicas enfocando el fuego o hacer que empiece a llamear el ascensor
end


function changeTimeDelayInOvenTrap(t_compname,value)
	t_compname.working = value
end

function  on_init_trap_oven_player()


	  
	

	h_oven1 =  GameController:entityByName("horno008");
	h_oven2 =  GameController:entityByName("horno009");
	h_oven3 =  GameController:entityByName("horno010");
	
	t_compSpawnOven1 = toCompEnemySpawner(toEntity(h_oven1):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven2 = toCompEnemySpawner(toEntity(h_oven2):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven3 = toCompEnemySpawner(toEntity(h_oven3):getCompByName("comp_enemy_spawner"));

	t_compSpawnOven1:setSpawnDelay(1.0);
	t_compSpawnOven2:setSpawnDelay(2.0);
	t_compSpawnOven3:setSpawnDelay(3.0);
	changeTimeDelayInOvenTrap(t_compSpawnOven1,false)
	changeTimeDelayInOvenTrap(t_compSpawnOven2,false)
	changeTimeDelayInOvenTrap(t_compSpawnOven3,false)



	execDelayedAction("changeTimeDelayInOvenTrap(t_compSpawnOven1,true)",0.1);
	execDelayedAction("changeTimeDelayInOvenTrap(t_compSpawnOven2,true)",1.1);
	execDelayedAction("changeTimeDelayInOvenTrap(t_compSpawnOven3,true)",2.1);



	execDelayedAction("on_delete_handle(\"triggerSpecialOven\")",0);

	--handle1 = GameController:entityByName("horno008");
	--t_compname1 = toCompEnemySpawnerSpecialTrap(toEntity(handle1):getCompByName("comp_enemy_spawner_special_trap"));
	--handle2 = GameController:entityByName("horno009");
	--t_compname2 = toCompEnemySpawnerSpecialTrap(toEntity(handle2):getCompByName("comp_enemy_spawner_special_trap"));
	--t_compname1:setSpawnDelay(1.0)
	--t_compname2:setSpawnDelay(2.0)
	--execDelayedAction("changeTimeDelayInOvenTrap(t_compname1)",0);
	--execDelayedAction("changeTimeDelayInOvenTrap(t_compname2)",1);
	--execDelayedAction("on_delete_handle(\"triggerSpecialOven\")",0);
	
end

--Mapa Tutorial panaderia







function activarSalidaPanaderia()

	GameController:resetCamera();
	execDelayedAction("on_cinematic(true)",0.0);
	execDelayedAction("on_lock_camera3(false)",0.0);
	execDelayedAction("on_blending_camera(\"CameraPanaderiaPlat\", 5,\"linear\")",0.1);
	--execDelayedAction("on_load_gpu_scene(\"data/scenes/mapa_congelados.json\")",11.5);
	--execDelayedAction("on_load_gpu_scene(\"data/particles/cold_particles.json\")",11.5);
	--execDelayedAction("on_load_gpu_scene(\"data/particles/smoke_cold_particles.json\")",5.5);
	--execDelayedAction("on_load_gpu_scene(\"data/particles/smoke_cold_particles2.json\")",6.5);
	--execDelayedAction("on_load_gpu_scene(\"data/particles/smoke_cold_particles3.json\")",6.5);
	--execDelayedAction("on_load_gpu_scene(\"data/particles/smoke_cold_particles4.json\")",7.5);
	--execDelayedAction("on_load_gpu_scene(\"data/particles/smoke_cold_particles5.json\")",8.5);
	--execDelayedAction("on_load_gpu_scene(\"data/particles/smoke_cold_particles6.json\")",9.5);
	execDelayedAction("on_blending_camera(\"PlayerCamera\",5,\"linear\")",13.5);
	execDelayedAction("on_lock_camera3(true)",18);
	execDelayedAction("on_cinematic(false)",18);
	


	execDelayedAction("activePlatformByName(\"plat1346469\")",5.2);
	execDelayedAction("activePlatformByName(\"plat1346470\")",7.2);
	execDelayedAction("activePlatformByName(\"plat1346471\")",9.2);


	h_oven1 =  GameController:entityByName("horno1");
	h_oven2 =  GameController:entityByName("horno5");
	h_oven3 =  GameController:entityByName("horno6");
	h_oven4 =  GameController:entityByName("horno007");

	
	t_compSpawnOven1 = toCompEnemySpawner(toEntity(h_oven1):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven2 = toCompEnemySpawner(toEntity(h_oven2):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven3 = toCompEnemySpawner(toEntity(h_oven3):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven4 = toCompEnemySpawner(toEntity(h_oven4):getCompByName("comp_enemy_spawner"));

	t_compSpawnOven1:setLifeSpawner(50.0);
	t_compSpawnOven2:setLifeSpawner(50.0);
	t_compSpawnOven3:setLifeSpawner(50.0);
	t_compSpawnOven4:setLifeSpawner(50.0);


	GameController:setLifeEnemiesByTag("cupcake",50.0);

end


function cinematica_fire_off()
	
	handlePlayer = GameController:getPlayerHandle();
	GameController:resetCamera();
	execDelayedAction("changeIntensityLightOmni(\"Omni018\",0.0)",0.0);
	execDelayedAction("on_lock_camera3(false)",0.0);
	execDelayedAction("on_blending_camera(\"CameraAscensorOn\", 5,\"Cubicinout\")",0.0);
	execDelayedAction("on_cinematic(true)",0.0);
	execDelayedAction("on_blending_camera(\"PlayerCamera\", 5,\"Cubicinout\")",7.0);
	execDelayedAction("on_lock_camera3(true)",12.0);
	execDelayedAction("on_cinematic(false)",12.0);

end





function cinematica_tower()
	
	handleCamera = GameController:entityByName("CameraTower");
	handlePlayer = GameController:getPlayerHandle();
	GameController:resetCamera();
	
	
	execDelayedAction("on_lock_camera3(false)",0.0);
	--execDelayedAction("on_delete_gpu_products()",7);

	execDelayedAction("on_blending_camera(\"CameraTower002\", 7,\"Cubicinout\")",0.0);

	execDelayedAction("on_blending_camera(\"CameraTower\", 7,\"Cubicinout\")",3.5);
	execDelayedAction("on_cinematic_special(true,1)",0.0);

	execDelayedAction("on_blending_camera(\"PlayerCamera\", 7,\"Cubicinout\")",10.0);
	execDelayedAction("on_lock_camera3(true)",15.7);
	execDelayedAction("on_cinematic_special(false,1)",15.7);
	--t_compCharacterController = toCompCharacterController(toEntity(handlePlayer):getCompByName("character_controller"));
	--t_compCharacterController:ChangeState("ESPECIAL_CINEMATIC");


end



function in_disable_throw_cupcakes_golem2_player()
	execDelayedAction("setNotThrowCupcake(\"golem2\",true)", 0);
	execDelayedAction("on_delete_handle(\"trigger009\")",0);
end

--Prueba carga de escena desde trigger, de momento carga NAVMESH
function changeScene(name)
	GameController:loadScene(name);
end


function changeGameState(name)
	GameController:changeGameState(name)
end

function deactivateWidget(nameWidget)
	GameController:deactivateWidget(nameWidget)
end

function activateWidget(nameWidget)
	GameController:activateWidget(nameWidget)
end

function childAppears(name,getFromChildren,alfaPos,valueIni,valueFin)
	GameController:childAppears(name,getFromChildren,alfaPos,valueIni,valueFin)
end


function stopWidgetEffect(nameWidget, effect)
	GameController:stopWidgetEffect(nameWidget, effect)
end

function changeSpeedWidgetEffect(nameWidget, effect, x , y)
	GameController:changeSpeedWidgetEffect(nameWidget, effect,x ,y )
end



function stopWidgetEffectSpecial()
	GameController:stopWidgetEffectSpecial()
end

function changeSpeedWidgetEffectSpecial(x,y)
	GameController:changeSpeedWidgetEffectSpecial(x,y)
end

function changeDurationWidgetEffectSpecial(duration)
	GameController:changeDurationWidgetEffectSpecial(duration)
end

function saveCheckpoint()
	handlePlayer = GameController:getPlayerHandle();
	t_compTranf = toCompTransform(toEntity(handlePlayer):getCompByName("transform"));
	pos = VEC3(t_compTranf:getPosition().x,t_compTranf:getPosition().y,t_compTranf:getPosition().z);
	rot = QUAT(t_compTranf:getRotation().x,t_compTranf:getRotation().y,t_compTranf:getRotation().z,t_compTranf:getRotation().w);
	GameController:saveCheckpoint(pos, rot);
end


function loadCheckpoint()
	GameController:loadCheckpoint();

end

function setEndGame()
	handlePlayer = GameController:getPlayerHandle();
	t_compcharac = toCompCharacterController_(toEntity(handlePlayer):getCompByName("character_controller"));
	t_compcharac.endgame = true 
end


function endGame()
	--GameController:resetCamera();
	execDelayedAction("resetCamera()",0.0);
	execDelayedAction("on_lock_camera3(false)",0.0);
	execDelayedAction("on_cinematic(true)",0.0);
	execDelayedAction("changeIntensityLightOmni(\"Omni038\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni039\",40.0)",15.5);
	
	execDelayedAction("changeIntensityLightOmni(\"Omni040\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni041\",40.0)",15.5);
	
	execDelayedAction("changeIntensityLightOmni(\"Omni042\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni043\",40.0)",15.5);

	execDelayedAction("changeIntensityLightOmni(\"Omni044\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni035\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni034\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni033\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni032\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni031\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni030\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni029\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni028\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni027\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni026\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni025\",40.0)",15.5);
	execDelayedAction("changeIntensityLightOmni(\"Omni024\",40.0)",15.5);



	execDelayedAction("on_blending_camera(\"CameraEndGame001\", 10,\"Cubicinout\")",0.0);
	execDelayedAction("on_blending_camera(\"CameraEndGame\", 10,\"Cubicinout\")",5.0);
	execDelayedAction("on_blending_camera(\"CameraEndGame002\", 10,\"Cubicinout\")",10.0);
	execDelayedAction("setEndGame()",15.0);


	--execDelayedAction("on_blending_camera(\"CameraEndGame\", 100,\"Cubicinout\")",0.0);
	execDelayedAction("activePlatformByName(\"globo\")",1);
	--execDelayedAction("changeGameState(\"gs_wingame\")",8);
end


function activateCurveForOven(nameOven,isScript,nameCurve)
	h_oven1 =  GameController:entityByName(nameOven);
	t_compSpawnOven1 = toCompEnemySpawner(toEntity(h_oven1):getCompByName("comp_enemy_spawner"));
	t_compSpawnOven1:setScriptTriggerActivate(isScript);
	t_compSpawnOven1:setCurveForSpawner(nameCurve);

end

function resetCamera()
	GameController:resetCamera();
end


function salCupcakeHorno010_008_player()
	
	execDelayedAction("resetCamera()",0.0);
	execDelayedAction("on_lock_camera3(false)",0.2);
	execDelayedAction("on_blending_camera(\"CameraCupcakeSalHorno\", 7,\"Cubicinout\")",0.2);
	execDelayedAction("on_cinematic(true)",0.0);

	execDelayedAction("on_blending_camera(\"PlayerCamera\", 7,\"Cubicinout\")",6.5);
	execDelayedAction("on_lock_camera3(true)",12);
	execDelayedAction("on_cinematic(false)",12);

	execDelayedAction("childAppears(\"MISION_1\",true,true,0.0,1.25)", 5.5)
		


	execDelayedAction("activateCurveForOven(\"horno008\",true,\"curvaCupcakeCongelados1\")",6);
	execDelayedAction("activateCurveForOven(\"horno010\",true,\"curvaCupcakeCongelados2\")",6.5);


	execDelayedAction("on_delete_handle(\"triggercinematicahornos\")",0.5);
	execDelayedAction("on_delete_handle(\"triggercinematicahornos2\")",0.5);
	execDelayedAction("saveCheckpoint()",6.6);

	playAnnouncement('event:/UI/Announcements/Announcement2')


end


function salCupcakeHorno010_008_2_player()

	execDelayedAction("resetCamera()",0.5);
	execDelayedAction("on_lock_camera3(false)",0.5);
	execDelayedAction("on_blending_camera(\"CameraCupcakeSalHorno\", 7,\"Cubicinout\")",0.5);
	execDelayedAction("on_cinematic(true)",0.5);

	execDelayedAction("childAppears(\"MISION_1\",true,true,0.0,1.25)", 5.5)

	execDelayedAction("on_blending_camera(\"PlayerCamera\", 7,\"Cubicinout\")",6.5);
	execDelayedAction("on_lock_camera3(true)",11);
	execDelayedAction("on_cinematic(false)",11);

	


	execDelayedAction("activateCurveForOven(\"horno008\",true,\"curvaCupcakeCongelados1\")",6);
	execDelayedAction("activateCurveForOven(\"horno010\",true,\"curvaCupcakeCongelados2\")",6.5);


	execDelayedAction("on_delete_handle(\"triggercinematicahornos\")",0.5);
	execDelayedAction("on_delete_handle(\"triggercinematicahornos2\")",0.5);
	execDelayedAction("saveCheckpoint()",6.6);

	playAnnouncement('event:/UI/Announcements/Announcement2')

end




function salCupcakeHorno008_player()

	

	--execDelayedAction("resetCamera()",0.5);
	--execDelayedAction("on_lock_camera3(false)",0.2);
	--execDelayedAction("on_blending_camera(\"CameraCupcakeSalHorno\", 7,\"Cubicinout\")",0.2);
	--execDelayedAction("on_cinematic(true)",0.2);
	--execDelayedAction("on_blending_camera(\"PlayerCamera\", 7,\"Cubicinout\")",5.5);
	--execDelayedAction("on_lock_camera3(true)",12);
	--execDelayedAction("on_cinematic(false)",12);
	playAnnouncement('event:/UI/Announcements/Announcement8')
	execDelayedAction("activateCurveForOven(\"horno008\",true,\"curvaHorno008\")",0.0);
	execDelayedAction("on_delete_handle(\"triggerSalCupcakeHorno008\")",0);
end



function salCupcakeHorno010_player()
	execDelayedAction("activateCurveForOven(\"horno010\",true,\"curvaHorno010\")",0.0);
	execDelayedAction("on_delete_handle(\"triggerSalCupcakeHorno010\")",0);

end

function salCupcakeHorno009_player()
	execDelayedAction("activateCurveForOven(\"horno009\",true,\"curvaHorno009\")",0.0);
	execDelayedAction("on_delete_handle(\"triggerSalCupcakeHorno009\")",0);
end

function playAnnouncement(announcement)
	GameController:playAnnouncement(announcement)
end

function setSoundtrackVolume(volume)
	GameController:setSoundtrackVolume(volume)
end

function startAudioPlayer(entityName)
	GameController:startAudioPlayer(entityName)
end

function balanceoLampara(entityName)
	h_lampara =  GameController:entityByName(entityName);
	t_lampara = toCompBalance(toEntity(h_lampara):getCompByName("comp_balance"));
	t_lampara:balanceo();
end

function apagarEncenderLuz(value)
	h_lampara =  GameController:entityByName("Joint001");
	t_lampara = toCompBalance(toEntity(h_lampara):getCompByName("comp_balance"));
	t_lampara:cambioTexturaJoint(value);
end


function resurrectionInGameOver()
	GameController:resurrectionInGameOver()
end

function exitGame()
	GameController:exitGame()
end

function setResurrect(value)
	GameController:setResurrect(value)
end

function setBloomInCam(value)
	GameController:setBloomInCam(value);
end

function pausedPlayer(value)
	GameController:pausedPlayer(value)
end

