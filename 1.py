import pygame
import random
import math

pygame.init()

WIDTH, HEIGHT = 900, 700
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Random Points Inside a 3D Sphere")

clock = pygame.time.Clock()

# -----------------------------
# Sphere parameters
# -----------------------------
SCALE = 250
CX = WIDTH // 2
CY = HEIGHT // 2

points = []

# -----------------------------
# Generate points
# -----------------------------
NUM_POINTS = 20000

for _ in range(NUM_POINTS):

    # 1. Choose x from -1 to +1
    x = random.uniform(-1, 1)

    # Radius of circle at this x
    y_limit = math.sqrt(1 - x*x)

    # 2. Choose y
    y = random.uniform(-y_limit, y_limit)

    # Radius available for z
    z_limit = math.sqrt(1 - x*x - y*y)

    # 3. Choose z
    z = random.uniform(-z_limit, z_limit)

    points.append((x, y, z))


# -----------------------------
# Rotation
# -----------------------------
angle_x = 0
angle_y = 0


def rotate_point(x, y, z):

    # Rotation around X axis
    cos_x = math.cos(angle_x)
    sin_x = math.sin(angle_x)

    y1 = y * cos_x - z * sin_x
    z1 = y * sin_x + z * cos_x

    # Rotation around Y axis
    cos_y = math.cos(angle_y)
    sin_y = math.sin(angle_y)

    x2 = x * cos_y + z1 * sin_y
    z2 = -x * sin_y + z1 * cos_y

    return x2, y1, z2


# -----------------------------
# Perspective projection
# -----------------------------
def project(x, y, z):

    # Camera distance
    camera = 3

    # Perspective
    factor = camera / (camera - z)

    screen_x = CX + x * SCALE * factor
    screen_y = CY - y * SCALE * factor

    return int(screen_x), int(screen_y), factor


# -----------------------------
# Main loop
# -----------------------------
running = True

while running:

    for event in pygame.event.get():

        if event.type == pygame.QUIT:
            running = False

        if event.type == pygame.KEYDOWN:

            if event.key == pygame.K_LEFT:
                angle_y -= 0.1

            if event.key == pygame.K_RIGHT:
                angle_y += 0.1

            if event.key == pygame.K_UP:
                angle_x -= 0.1

            if event.key == pygame.K_DOWN:
                angle_x += 0.1

    screen.fill((0, 0, 0))

    # Rotate and project points
    projected_points = []

    for x, y, z in points:

        x1, y1, z1 = rotate_point(x, y, z)

        sx, sy, factor = project(x1, y1, z1)

        projected_points.append((z1, sx, sy, factor))

    # Draw far points first
    projected_points.sort()

    for z, sx, sy, factor in projected_points:

        if 0 <= sx < WIDTH and 0 <= sy < HEIGHT:

            # Slightly bigger points when closer
            radius = max(1, int(1.5 * factor))

            pygame.draw.circle(
                screen,
                (255, 255, 255),
                (sx, sy),
                radius
            )

    pygame.display.flip()

    clock.tick(60)

pygame.quit()