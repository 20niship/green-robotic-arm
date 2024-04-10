import torch
import numpy as np
import glob
from torch import nn
import matplotlib.pyplot as plt
from typing import List
from torchvision.transforms import ToTensor

from dataloader import MyDataset

DATASET_PATH = "exp2"
dataset = MyDataset("dataset/" + DATASET_PATH)

batch_size = 32

train_dataloader = torch.utils.data.DataLoader(
     dataset=dataset,
     batch_size=batch_size,
     shuffle=False,
     # num_workers=2
)

def predict_trajectory(model, default_pos: List[float]):
    res : List[List[float]] = []
    res.append(default_pos)
    now = np.array([default_pos], dtype=np.float32)
    for i in range(100):
        y = model(torch.tensor(now))
        res.append(y.tolist()[0])
        now = y
    return res

def _get_output_size()->int:
    for a in train_dataloader:
        return len(a[0][0])
    return 0

OUTPUT_SIZE = _get_output_size()

class NeuralNetwork(nn.Module):
    def __init__(self):
        super().__init__()
        self.flatten = nn.Flatten()
        self.linear_relu_stack = nn.Sequential(
            nn.Linear(OUTPUT_SIZE, 512),
            # nn.ReLU(),
            # nn.Linear(512, 512),
            nn.ReLU(),
            nn.Linear(512, OUTPUT_SIZE),
        )

    def forward(self, x):
        x = self.flatten(x)
        logits = self.linear_relu_stack(x)
        return logits

model = NeuralNetwork()
model.load_state_dict(torch.load('model_weight' + DATASET_PATH + '.pth'))
model.eval()


def inout_to_xyz(x:List[float]):
    return [x[7], x[9], x[11], x[13], x[15], x[17]]


def get_answer():
    dataset = np.loadtxt("exp2.csv", delimiter=',', skiprows=1)
    data = dataset
    return data[0], data

def main():
    x_list, answer = get_answer()

    y = predict_trajectory(model, x_list)
    predicted = []
    for y_ in y:
        predicted.append(inout_to_xyz(y_))

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # x_pred = [x[0] for x in predicted]
    # y_pred = [x[1] for x in predicted]
    # z_pred = [x[2] for x in predicted]
    x_pred = [x[3] for x in predicted]
    y_pred = [x[4] for x in predicted]
    z_pred = [x[5] for x in predicted]
    ax.plot(x_pred, y_pred, z_pred, color="red")


    answer = [inout_to_xyz(a) for a in answer]
    # x_ans = [x[0] for x in answer]
    # y_ans = [x[1] for x in answer]
    # z_ans = [x[2] for x in answer]
    x_ans = [x[3] for x in answer]
    y_ans = [x[4] for x in answer]
    z_ans = [x[5] for x in answer]
    ax.plot(x_ans, y_ans, z_ans, color="blue")

    plt.show()


if __name__ == "__main__":
    main()

