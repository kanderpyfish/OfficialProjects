import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Calendar;
import java.text.SimpleDateFormat;

public class MyDate {

    private int year;
    private int month;
    private int day;



    public MyDate(int year, int month, int day) throws IllegalArgumentException {

        if (!isValid(year, month, day)) throw new IllegalArgumentException();
        this.year = year;
        this.month = month;
        this.day = day;     
    }

    public MyDate() {
        GregorianCalendar gcal = new GregorianCalendar();
        this.year = gcal.get

        Calendar currentDate = Calendar.getInstance(); //Get the current date
        java.util.Date x = currentDate.getTime();
        SimpleDateFormat formatyear = new SimpleDateFormat("yyyy");
        this.year = Integer.parseInt(formatyear.format(x));
        SimpleDateFormat formatmonth = new SimpleDateFormat("MM");
        this.month = Integer.parseInt(formatmonth.format(x));     
        SimpleDateFormat formatdd = new SimpleDateFormat("dd");
        this.day = Integer.parseInt(formatdd.format(x));       
    }


    public static boolean isLeap(int year) {
        // using system library to do this, avoid re-invent the wheel
        Calendar cal = Calendar.getInstance();
        cal.set(Calendar.YEAR, year);
        return cal.getActualMaximum(Calendar.DAY_OF_YEAR) > 365;     
    }



    public static boolean isValid(int year, int month, int day) {
        if (year < 0) return false;
        if ((month > 11)) return false;
        if ((day < 1) || (day > 31)) return false;
        switch (month) {
            case 0: return true;
            case 1: return (isLeap(year) ? day <= 29 : day <= 28);
            case 2: return true;
            case 3: return day < 31;
            case 4: return true;
            case 5: return day < 31;
            case 6: return true;
            case 7: return true;
            case 8: return day < 31;
            case 9: return true;
            case 10: return day < 31;
            default: return true;
        }
    }

    


    public static void main(String[] args)  {

        String month[] = { "Jan", "Feb", "Mar", "Apr",
                           "May", "Jun", "Jul", "Aug",
                           "Sep", "Oct", "Nov", "Dec" };

        GregorianCalendar gcal = new GregorianCalendar();

        gcal.setTimeInMillis(1000000000);

        
 
        // displaying the date, time, time zone and locale
        System.out.print("Date: "
                         + month[gcal.get(Calendar.MONTH)] + " "
                         + gcal.get(Calendar.DATE) + ", "
                         + gcal.get(Calendar.YEAR) + "\n");
                         
    }
    
}
