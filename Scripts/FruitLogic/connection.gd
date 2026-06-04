extends Area2D

func _ready():
	area_entered.connect(_on_area_entered)

func _on_area_entered(area: Area2D) -> void:
	if area.is_in_group("player_catcher"):
		var spawner = get_tree().get_first_node_in_group("fruit_spawner")
		if spawner:
			spawner.call("fruit_caught", self)
		queue_free()
