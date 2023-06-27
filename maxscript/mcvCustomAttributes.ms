clearListener()

classPrefabData = attributes prefabData
(
	parameters main rollout:params
	(
		prefab type: #integer ui:ddlPrefab default:1
	)
	rollout params "Weapon Parameters" width:163 height:168
	(
		dropDownList 'ddlPrefab' "DropDownList" pos:[10,10] width:151 height:40 items:#(
			"Player", 
			"Carrito", 
			"Spawner Teletransportable",
			"Cupcake", 
			"Sushi",
			"Golem", 
			"Spawner Enemigos",
			"Caja Explosiva",
			"Muro de hielo",
			"Fuente de locura",
			"Charco de locura",
			"Power Up de Vida",
			"Power Up de Locura", 
			"Checkpoint",
			"Sushi Arquero", 
			"Spawner Dumpling",
			"Termostato",
			"Scanner",
			"Pila",
			"Extintor",
			"Cafe",
			"Teleport Object",
			"Trap_air",
			"Trap_carniceria",
			"Trap_tube_enemies",
			"Trap_butcher_enemies",
			"Trap_special_oven",
			"Fire trap",
			"Player Camera",
			"Joint Light"
			) align:#left
	)
	fn exportAsComponent fs = (
		local src = "";
		if prefab == 1 then src = "data/prefabs/character/player.json"
		else if prefab == 2 then src = "data/prefabs/powers/sCart.json"
		else if prefab == 3 then src = "data/prefabs/structures/teleportable_object_spawner.json"
		else if prefab == 4 then src = "data/prefabs/enemies/bt_cupcake.json"
		else if prefab == 5 then src = "data/prefabs/enemies/bt_sushi.json"
		else if prefab == 6 then src = "data/prefabs/enemies/bt_golem.json"
		else if prefab == 7 then src = "data/prefabs/structures/enemy_spawner.json"
		else if prefab == 8 then src = "data/prefabs/structures/explosive_object.json"
		else if prefab == 9 then src = "data/prefabs/structures/wall.json"
		else if prefab == 10 then src = "data/prefabs/structures/madness_fountain.json"
		else if prefab == 11 then src = "data/prefabs/structures/madness_puddle.json"
		else if prefab == 12 then src = "data/prefabs/powers/powerUp_health.json"
		else if prefab == 13 then src = "data/prefabs/powers/powerUp_madness.json"
		else if prefab == 14 then src = "data/prefabs/structures/checkpoint.json"
		else if prefab == 15 then src = "data/prefabs/enemies/bt_ranged_sushi.json"
		else if prefab == 16 then src = "data/prefabs/structures/spawner_dumpling.json"
		else if prefab == 17 then src = "data/prefabs/structures/termostate.json"
		else if prefab == 18 then src = "data/prefabs/powers/powerUp_scanner.json"
		else if prefab == 19 then src = "data/prefabs/powers/powerUp_pila.json"
		else if prefab == 20 then src = "data/prefabs/powers/powerUp_extintor.json"
		else if prefab == 21 then src = "data/prefabs/powers/powerUp_cafe.json"
		else if prefab == 22 then src = "data/prefabs/structures/teleport_box.json"
		else if prefab == 23 then src = "data/prefabs/traps/wind_trap.json"
		else if prefab == 24 then src = "data/prefabs/traps/blade_trap.json"
	    else if prefab == 25 then src = "data/prefabs/traps/enemies_in_tube.json"
		else if prefab == 26 then src = "data/prefabs/traps/enemies_in_butcher.json"		
		else if prefab == 27 then src = "data/prefabs/structures/enemy_spawner_special.json"
		else if prefab == 28 then src = "data/prefabs/structures/fire_trap.json"
		else if prefab == 29 then src = "data/prefabs/cameras/player_camera.json"
		else if prefab == 30 then src = "data/prefabs/lights/light_joint.json"
		else return undefined
		fs.writeComma()
		fs.writeKeyValue "prefab"  src
	)
)

fn createEmtyPrefab = (
	local newDummy = Dummy()
	custAttributes.add newDummy classPrefabData
	select newDummy
)
