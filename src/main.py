from room import Position, FarAxisOrigin2DPositionCalculator

def main():
    print("Hello position calculator")
    origin = Position(0, 0, 0)
    far_x = Position(21.2, 0, 0)

    another_far_x = Position(21.2, 0, 0)

    print(origin.get_dist_to(far_x))
    print(far_x.is_equal(another_far_x))

    origin = Position(0, 0, 0)
    tag1_pos = Position(13, 10, 7)
    far_x1_pos = Position(22, 0, 0)
    far_y1_pos = Position(0, 3, 0)
    origin_to_far_x1 = origin.get_dist_to(far_x1_pos)
    origin_to_far_y1 = origin.get_dist_to(far_y1_pos)
    tag1_to_origin = tag1_pos.get_dist_to(origin)
    tag1_to_far_x1 = tag1_pos.get_dist_to(far_x1_pos)
    tag1_to_far_y1 = tag1_pos.get_dist_to(far_y1_pos)
    tag1_dist_anchor_pos = [
        (tag1_to_origin, origin),
        (tag1_to_far_x1, far_x1_pos),
        (tag1_to_far_y1, far_y1_pos),
    ]

    position_cal = FarAxisOrigin2DPositionCalculator()
    calculated_tag1_pos = position_cal.get_position_or_null_position(tag1_dist_anchor_pos)  
    print(calculated_tag1_pos.to_string())
   

if __name__ == "__main__":
    main()