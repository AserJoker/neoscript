/**
 * @author aserjoker@outlook.com
 */
import data from 'demo' assert {type: "js"}/*load data as js */;
const str = `data is:${{ data: data }} value is :${data}\n`;
console.log(`test result is:${/.*/.test(str)}`); //test data