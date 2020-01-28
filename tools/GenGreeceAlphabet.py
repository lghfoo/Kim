# alpha
# /'ælfə/
# Α
# α
# nu
# /nju:/
# Ν
# ν
# beta
# /'bi:tə/或 /'beɪtə/
# Β
# β
# xi
# 希腊 /ksi/；英美 /ˈzaɪ/ 或 /ˈksaɪ/
# Ξ
# ξ
# gamma
# /'gæmə/
# Γ
# γ
# omicron
# /əuˈmaikrən/或 /ˈɑmɪˌkrɑn/
# Ο
# ο
# delta
# /'deltə/
# Δ
# δ
# pi
# /paɪ/
# Π
# π
# epsilon
# /'epsɪlɒn/
# Ε
# ε
# rho
# /rəʊ/
# Ρ
# ρ
# zeta
# /'zi:tə/
# Ζ
# ζ
# sigma
# /'sɪɡmə/
# Σ
# σ ς
# eta
# /'i:tə/
# Η
# η
# tau
# /tɔ:/ 或 /taʊ/
# Τ
# τ
# theta
# /'θi:tə/
# Θ
# θ
# upsilon
# /ˈipsilon/或 /ˈʌpsɨlɒn/
# Υ
# υ
# iota
# /aɪ'əʊtə/	
# Ι
# ι ℩
# phi
# /faɪ/	
# Φ
# φ
# kappa
# /'kæpə/	
# Κ
# κ
# chi
# /kaɪ/	
# Χ
# χ
# lambda
# /'læmdə/	
# Λ
# λ
# psi
# /psaɪ/	
# Ψ
# ψ
# mu
# /mju:/	
# Μ
# μ
# omega
# /'əʊmɪɡə/或 /oʊ'meɡə/
# Ω
# ω
# f = open("./tmp.txt", encoding='UTF-8')
# lines = f.readlines()
# letter_left = []
# letter_right = []
# flag = 0
# count = 0
# tmp_tuple = ()
# for line in lines:
#     count += 1
#     tmp_tuple += (line.replace('\n', '').strip(),)
#     if count % 4 == 0:
#         if flag % 2 == 0:
#             letter_left.append(tmp_tuple)
#         else:
#             letter_right.append(tmp_tuple)
#         tmp_tuple = ()
#         flag += 1
# f.close();

# for letter in letter_left:
#     print('{' + ('"{}", "{}", "{}", "{}"'.format(letter[0], letter[1], letter[2], letter[3])) + '},')
# for letter in letter_right:
#     print('{' + ('"{}", "{}", "{}", "{}"'.format(letter[0], letter[1], letter[2], letter[3])) + '},')

letter_lines = '''Α
α
/'ælfə/
alpha	
阿尔法	
角度、系数、角加速度、第一个、电离度、转化率
Β	
β	
/'bi:tə/ 或 /'beɪtə/
beta	
贝塔
磁通系数、角度、系数
Γ	
γ	
/'gæmə/	
gamma	
伽玛
电导系数、角度、比热容比
Δ	
δ	
/'deltə/	
delta	
得尔塔/徳尔塔
变化量、焓变、熵变、屈光度、一元二次方程中的判别式、化学位移
Ε	
ε	
/'epsɪlɒn/	
epsilon	
艾普西隆	
对数之基数、介电常数、电容率、应变
Ζ	
ζ	
/'zi:tə/	
zeta	
泽塔	
系数、方位角、阻抗、相对黏度
Η	
η	
/'i:tə/	
eta	
伊塔
迟滞系数、机械效率
Θ	
θ	
/'θi:tə/	
theta	
西塔	
温度、角度
Ι	
ι	
/aɪ'əʊtə/	
iota	
约(yāo)塔	
微小、一点
Κ	
κ	
/'kæpə/	
kappa	
卡帕	
介质常数、绝热指数
∧	
λ	
/'læmdə/	
lambda	
拉姆达	
波长、体积、导热系数、普朗克常数
Μ	
μ	
/mju:/	
mu	
谬	
磁导率、微、动摩擦系（因）数、流体动力黏度、货币单位、莫比乌斯函数
Ν	
ν	
/nju:/	
nu	
纽	
磁阻系数、流体运动粘度、光波频率、化学计量数
Ξ	
ξ	
希腊 /ksi/ 英美 /ˈzaɪ/ 或 /ˈsaɪ/
xi	
克西
随机变量、（小）区间内的一个未知特定值
Ο	
ο	
/əuˈmaikrən/ 或 /ˈɑmɪˌkrɑn/
omicron	
奥米克戎	
高阶无穷小函数
∏	
π	
/paɪ/	
pi	
派	
圆周率、π(n)表示不大于n的质数个数、连乘
Ρ	
ρ	
/rəʊ/	
rho	
柔	
电阻率、柱坐标和极坐标中的极径、密度、曲率半径
∑	
σ,ς	
/'sɪɡmə/	
sigma	
西格马	
总和、表面密度、跨导、应力、电导率	
Τ	
τ	
/tɔ:/ 或 /taʊ/
tau	
陶	
时间常数、切应力、2π（两倍圆周率）
Υ	
υ	
/ˈipsɪlon/ 或 /ˈʌpsɪlɒn/
upsilon	
阿普西龙	
位移
Φ	
φ	
/faɪ/	
phi	
斐
磁通量、电通量、角、透镜焦度、热流量、电势、直径、欧拉函数、空集
Χ	
χ	
/kaɪ/	
chi	
希	
统计学中有卡方(χ^2)分布	
Ψ
ψ	
/psaɪ/	
psi	
普西	
角速、介质电通量、ψ函数、磁链
Ω	
ω	
/'əʊmɪɡə/ 或 /oʊ'meɡə/
omega	
奥米伽/欧米伽
欧姆、角速度、角频率、交流电的电角度、化学中的质量分数、有机物的不饱和度
'''

lines = letter_lines.splitlines()
letters = []
flag = 0
count = 0
tmp_tuple = ()
for line in lines:
    count += 1
    tmp_tuple += (line.replace('\n', '').strip(),)
    if count % 6 == 0:
        letters.append(tmp_tuple)
        tmp_tuple = ()
for letter in letters:
    print('' + ('"{}", "{}", "{}", "{}", "{}", "{}"'.format(letter[0], letter[1], letter[2], letter[3], letter[4], letter[5])) + '')