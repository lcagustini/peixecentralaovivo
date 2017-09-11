function angle(x, y, z){
    let xa = Math.atan(x/z);
    let ya = Math.asin(y);

    if(x < 0)
        xa += Math.PI;

    return [xa, ya];
}

console.log(angle(1,0.01,0.01));
console.log(angle(0.01,1,0.01));
console.log(angle(0.01,0.01,1));
