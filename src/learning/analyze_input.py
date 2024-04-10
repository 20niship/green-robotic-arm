import torch
from torch.autograd import Variable
import numpy as np
import pickle
import sys
from torch import nn
import matplotlib.pyplot as plt
from typing import List

batch_size = 32

OUTPUT_SIZE = 12
labels = [
        "sensor0",  "sensor1",  "sensor2",  "sensor3",  "sensor4",  "sensor5",
        "axis0_pos", "axis0_vel", "axis1_pos", "axis1_vel", "axis2_pos", "axis2_vel",
        "axis3_pos", "axis3_vel", "axis4_pos", "axis4_vel", "axis5_pos", "axis5_vel",
        ]


class NeuralNetwork(nn.Module):
    def __init__(self):
        super().__init__()
        self.flatten = nn.Flatten()
        self.linear_relu_stack = nn.Sequential(
            nn.Linear(OUTPUT_SIZE, 512),
            nn.ReLU(),
            nn.Linear(512, 256),
            nn.ReLU(),
            nn.Linear(256, 6),
        )

    def forward(self, x):
        x = self.flatten(x)
        logits = self.linear_relu_stack(x)
        return logits

model = NeuralNetwork()
model.load_state_dict(torch.load('model_weight' + "exp2"+ '.pth'))
model.eval()

filepath = sys.argv[1]

def main2(task_index: int = 5):
    """
    ある一つのタスクについて一連の入力に対するモデルの勾配を可視化
    """
    db = pickle.load(open(filepath, "rb"))
    raw = db["data_raw"]
    input_data = np.array(raw, dtype=np.float32)
    print(input_data.shape)
    a = np.where(input_data[:,0] == task_index)
    input_data = input_data[a]
    input_data = np.delete(input_data, 0, 1)
    print(input_data.shape)
    xyz_index = np.array([6,7,8,9,10,11]) 
    xyz = input_data[:, xyz_index]

    def get_input_gradients(model, input_data, target_class):
        input_data = torch.tensor(input_data, dtype=torch.float32)
        model.eval()
        input_data = Variable(input_data, requires_grad=True)
        output = model(input_data)
        loss = output[0, target_class]  # 任意の出力クラスに対する損失
        loss.backward()
        input_gradients = input_data.grad.data
        return input_gradients.numpy()

    def get_model_gradients(model, input_data):
        return [get_input_gradients(model, input_data, i) for i in range(6)]

    def predicted(model, input_data):
        input_data_ = torch.tensor(input_data, dtype=torch.float32)
        return model(input_data_).detach().numpy()

    pred = predicted(model, input_data)
    grads = get_model_gradients(model, input_data)
    # 2x3のグラフを作成
    fig, axes = plt.subplots(2, 3)
    # 2x3のグラフにそれぞれ描画
    for i, ax in enumerate(axes.flatten()):
        ax.plot(pred[:, i], label="pred")
        ax.plot(xyz[:, i], label="actual")
        g = grads[i][:i]
        print(g)
        print(g.shape)
        ax.plot(g, label="grad")
        ax.set_title("J" + str(i))
        ax.legend()
        ax.grid()
    plt.show()

def main3():
    db = pickle.load(open(filepath, "rb"))
    input_data = np.array(db["data_raw"])
    a = np.where(input_data[:,0] == 7)
    input_data = input_data[a]
    input_data = np.delete(input_data, 0, 1)
    print(input_data.shape)
    xyz_index = np.array([6,7,8,9,10,11])
    input_data = input_data[:, xyz_index]
    plt.plot(input_data)
    plt.show()

main2()

# plt.bar( range(len(grasd)), grasd, tick_label=labels)
# plt.show()

plt.imshow(gradients.squeeze(), cmap='viridis')
plt.show()
