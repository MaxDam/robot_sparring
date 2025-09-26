#!/usr/bin/env python3
"""
movenet_coral_camera.py

Cattura un singolo frame dalla camera, esegue MoveNet su Google Coral (Edge TPU)
e salva/mostra un'immagine con i punti di posa sovrapposti.

Usage example:
    python3 movenet_coral_camera.py --model /path/to/movenet_edgetpu.tflite --camera 0 --out snapshot.png

Requisiti:
    pip install pycoral opencv-python numpy

Nota: usare un modello compilato per Edge TPU (es. movenet_singlepose_lightning_edgetpu.tflite).
"""

import argparse
import sys
import cv2
import numpy as np
from pycoral.utils.edgetpu import make_interpreter
from pycoral.adapters import common

# Mappatura keypoints (MoveNet - 17 punti)
KEYPOINT_NAMES = [
    "nose", "left_eye", "right_eye", "left_ear", "right_ear",
    "left_shoulder", "right_shoulder", "left_elbow", "right_elbow",
    "left_wrist", "right_wrist", "left_hip", "right_hip",
    "left_knee", "right_knee", "left_ankle", "right_ankle"
]

# Connessioni per disegnare lo scheletro (indice basato su KEYPOINT_NAMES)
SKELETON = [
    (0, 1), (0, 2), (1, 3), (2, 4),
    (0, 5), (0, 6), (5, 7), (7, 9), (6, 8), (8, 10),
    (5, 6), (11, 12), (11, 13), (13, 15), (12, 14), (14, 16)
]


def parse_args():
    p = argparse.ArgumentParser(description='MoveNet + Coral: scatta un frame e disegna i punti di posa')
    p.add_argument('--model', '-m', required=True, help='Percorso al modello .tflite compilato per Edge TPU')
    p.add_argument('--camera', '-c', type=int, default=0, help='Indice della camera (default 0)')
    p.add_argument('--out', '-o', default='snapshot_pose.png', help='Nome file di output per l'immagine con i punti')
    p.add_argument('--min-score', type=float, default=0.3, help='Soglia minima di confidenza per disegnare un keypoint')
    return p.parse_args()


def load_interpreter(model_path):
    interp = make_interpreter(model_path)
    interp.allocate_tensors()
    return interp


def run_movenet_on_image(interpreter, image_rgb):
    """Esegue l'inferenza MoveNet sul frame RGB (numpy array) e ritorna i keypoints normalizzati.

    Ritorna un array (17,3) con (y, x, score) normalizzati [0..1].
    """
    input_size = common.input_size(interpreter)
    # Ridimensiona senza mantenere ratio (MoveNet richiede input squadrato della dimensione del modello)
    resized = cv2.resize(image_rgb, input_size)

    # Common helper per settare l'input correttamente (dtype atteso dal modello)
    common.set_input(interpreter, resized)
    interpreter.invoke()

    out_details = interpreter.get_output_details()
    out_idx = out_details[0]['index']
    output_data = interpreter.get_tensor(out_idx)

    # MoveNet singlepose solitamente produce (1,1,17,3) o (1,17,3)
    if output_data.ndim == 4:
        keypoints = output_data[0][0]
    elif output_data.ndim == 3:
        keypoints = output_data[0]
    else:
        raise RuntimeError(f'Formato output inatteso: shape={output_data.shape}')

    return keypoints


def draw_keypoints_on_image(image, keypoints, min_score=0.3):
    h, w = image.shape[:2]
    img = image.copy()

    # Converti coordinate normalizzate (y,x) in pixel sulla immagine originale
    pts = []
    for i, kp in enumerate(keypoints):
        y, x, score = float(kp[0]), float(kp[1]), float(kp[2])
        px = int(x * w)
        py = int(y * h)
        pts.append((px, py, score))

    # Disegna linee dello scheletro
    for a, b in SKELETON:
        ax, ay, ascore = pts[a]
        bx, by, bscore = pts[b]
        if ascore >= min_score and bscore >= min_score:
            cv2.line(img, (ax, ay), (bx, by), (0, 255, 0), 2)

    # Disegna keypoints
    for i, (px, py, score) in enumerate(pts):
        if score >= min_score:
            cv2.circle(img, (px, py), 4, (0, 0, 255), -1)
            cv2.putText(img, f"{KEYPOINT_NAMES[i]}:{score:.2f}", (px+5, py-5), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255,255,255), 1, cv2.LINE_AA)

    return img


def main():
    args = parse_args()

    print('Caricamento modello:', args.model)
    interpreter = load_interpreter(args.model)
    in_w, in_h = common.input_size(interpreter)
    print(f'Modello input size: {in_w}x{in_h}')

    cap = cv2.VideoCapture(args.camera)
    if not cap.isOpened():
        print('Errore: impossibile aprire la camera', file=sys.stderr)
        sys.exit(1)

    # Cattura un singolo frame (istantanea)
    ret, frame = cap.read()
    cap.release()
    if not ret:
        print('Errore: impossibile leggere frame dalla camera', file=sys.stderr)
        sys.exit(1)

    # OpenCV restituisce BGR: converti in RGB
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    keypoints = run_movenet_on_image(interpreter, frame_rgb)
    annotated = draw_keypoints_on_image(frame, keypoints, min_score=args.min_score)

    # Salva e mostra
    cv2.imwrite(args.out, annotated)
    print(f'Immagine salvata in: {args.out}')

    # Mostra finestra per qualche secondo
    cv2.imshow('Pose snapshot', annotated)
    print('Premi un tasto nella finestra per chiudere...')
    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()
